import os, sys, rospy
import numpy as np
import cPickle as pickle
from collections import deque
from joblib import Parallel, delayed

import hrl_lib.util as ut
from hrl_anomaly_detection import util
from hrl_anomaly_detection import data_manager as dm
from hrl_anomaly_detection.ICRA2017_params import *

# learning
from hrl_anomaly_detection.hmm import learning_hmm
from hrl_anomaly_detection.hmm import learning_util as hmm_util
from sklearn import preprocessing

# Classifier
from hrl_anomaly_detection.classifiers import classifier as clf
from hrl_anomaly_detection.classifiers.classifier_util import *



class WeightSearch:
    def __init__(self):
        self.initParams()
        # Load HMM and Training/Test Data
        self.loadAll()

        # Preprocess all test data
        self.preprocessData()
        self.evalClassifier(0.75)


    def initParams(self):
        print 'Loading and initializing all parameters'
        self.hmm_renew = False
        self.sgd_n_iter = 100
        self.startIdx  = 4

        self.nTests = 20
        self.ll_test_X = deque([],self.nTests)
        self.ll_test_Y = deque([],self.nTests)

        self.task_name = 'scooping'
        dim = 4
        rf_center = 'kinEEPos'
        local_range = 10.0
        self.raw_data_path, self.save_data_path, self.param_dict = getParams(self.task_name, False, False, False, dim,
                                                                             rf_center, local_range, nPoints=10)
        if self.task_name == 'scooping':
            self.subject_names = ['park', 'new']
            self.classifier_method = 'sgd'
            self.save_data_path = os.path.expanduser('~')+'/hrl_file_server/dpark_data/anomaly/ICRA2017/max_accuracy/'+self.task_name
            self.param_dict['SVM'] = {'renew': False, 'w_negative': 4.0, 'gamma': 0.04, 'cost': 4.6,
                         'class_weight': 1.5e-2, 'logp_offset': 0, 'ths_mult': -2.0,
                         'sgd_gamma':0.32, 'sgd_w_negative':2.5,}

            self.param_dict['data_param']['nNormalFold']   = 1
            self.param_dict['data_param']['nAbnormalFold'] = 1
            self.param_dict['AD']['eval_target'] = ['ref']

        self.rf_radius = self.param_dict['data_param']['local_range']
        self.rf_center = self.param_dict['data_param']['rf_center']
        self.downSampleSize = self.param_dict['data_param']['downSampleSize']
        self.handFeatures = self.param_dict['data_param']['handFeatures']
        self.cut_data     = self.param_dict['data_param']['cut_data']
        self.nNormalFold   = self.param_dict['data_param']['nNormalFold']
        self.nAbnormalFold = self.param_dict['data_param']['nAbnormalFold']

        self.nState = self.param_dict['HMM']['nState']
        self.cov    = self.param_dict['HMM']['cov']
        self.scale  = self.param_dict['HMM']['scale']
        self.add_logp_d = self.param_dict['HMM'].get('add_logp_d', True)

        self.SVM_dict        = self.param_dict['SVM']
        self.w_max = self.param_dict['ROC'][self.classifier_method+'_param_range'][-1]
        self.w_min = self.param_dict['ROC'][self.classifier_method+'_param_range'][0]
        self.exp_sensitivity = True


    def loadAll(self):
        'Beginning to prepare HMM and datasets'
        self.hmm_model_pkl = os.path.join(self.save_data_path, 'hmm_'+self.task_name + '.pkl')

        (success_list, failure_list) = util.getSubjectFileList(self.raw_data_path, self.subject_names, self.task_name, time_sort=True)
        self.used_file_list = success_list+failure_list

        if os.path.isfile(self.hmm_model_pkl) and self.hmm_renew is False:
            self.loadHMM()
        else:
            self.loadData()
            self.trainHMM()
            self.prepareClassifierData()
            self.storeHMM()


    def loadHMM(self):
        print 'HMM and process data stored in file. Loading everything into memory'
        d = ut.load_pickle(self.hmm_model_pkl)
        # HMM
        self.nEmissionDim = d['nEmissionDim']
        self.A            = d['A']
        self.B            = d['B']
        self.pi           = d['pi']
        self.ml = learning_hmm.learning_hmm(self.nState, self.nEmissionDim, verbose=False)
        self.ml.set_hmm_object(self.A, self.B, self.pi)

        self.ll_classifier_train_X = d['ll_classifier_train_X']
        self.ll_classifier_train_Y = d['ll_classifier_train_Y']
        self.X_train_org   = d['X_train_org']
        self.Y_train_org   = d['Y_train_org']
        self.idx_train_org = d['idx_train_org']
        self.nLength       = d['nLength']
        self.handFeatureParams = d['param_dict']
        self.normalTrainData   = d.get('normalTrainData', None)


    def storeHMM(self):
        print 'Saving HMM and data to file'
        d                  = {}
        d['A']             = self.ml.A
        d['B']             = self.ml.B
        d['pi']            = self.ml.pi
        d['nEmissionDim']  = self.nEmissionDim
        d['ll_classifier_train_X'] = self.ll_classifier_train_X
        d['ll_classifier_train_Y'] = self.ll_classifier_train_Y
        d['X_train_org']   = self.X_train_org
        d['Y_train_org']   = self.Y_train_org
        d['idx_train_org'] = self.idx_train_org
        d['nLength']       = self.nLength = len(self.normalTrainData[0][0])
        d['param_dict']    = self.handFeatureParams
        d['normalTrainData'] = self.normalTrainData
        ut.save_pickle(d, self.hmm_model_pkl)


    def loadData(self):
        print 'Loading training and validation datasets'
        dd = dm.getDataSet(self.subject_names, self.task_name, self.raw_data_path, self.save_data_path, self.rf_center, self.rf_radius,
                           downSampleSize=self.downSampleSize, scale=1.0, ae_data=False, handFeatures=self.handFeatures,
                           cut_data=self.cut_data, data_renew=False, time_sort=True)

        self.handFeatureParams = dd['param_dict']

        # do we need folding????????????????
        if self.nNormalFold > 1:
            # Task-oriented hand-crafted features
            kFold_list = dm.kFold_data_index2(len(dd['successData'][0]), len(dd['failureData'][0]), self.nNormalFold, self.nAbnormalFold)
            # Select the first fold as the training data (need to fix?)
            (normalTrainIdx, abnormalTrainIdx, normalTestIdx, abnormalTestIdx) = kFold_list[0]
        else:
            #TODO?
            normalTrainIdx   = range(len(dd['successData'][0]))
            abnormalTrainIdx = range(len(dd['failureData'][0]))
            normalTestIdx   = None
            abnormalTestIdx = None

        # dim x sample x length # TODO: what is the best selection?
        self.normalTrainData   = dd['successData'][:, normalTrainIdx, :]   * self.scale
        self.abnormalTrainData = dd['failureData'][:, abnormalTrainIdx, :] * self.scale


    def trainHMM(self):
        print 'Creating and fitting HMM'
        self.nEmissionDim = len(self.normalTrainData)
        self.ml = learning_hmm.learning_hmm(self.nState, self.nEmissionDim, verbose=False)
        if self.param_dict['data_param']['handFeatures_noise']:
            ret = self.ml.fit(self.normalTrainData+np.random.normal(0.0, 0.03, np.shape(self.normalTrainData))*self.scale,
                              cov_mult=[self.cov]*(self.nEmissionDim**2))
        else:
            ret = self.ml.fit(normalTrainData, cov_mult=[self.cov]*(self.nEmissionDim**2))

        if ret == 'Failure':
            print '-'*25, '\nHMM training failed.\n', '-'*25
            sys.exit()


    def prepareClassifierData(self):
        print 'Preparing data for classifier'
        trainDataX = []
        trainDataY = []
        for i in xrange(self.nEmissionDim):
            temp = np.vstack([self.normalTrainData[i], self.abnormalTrainData[i]])
            trainDataX.append(temp)

        trainDataY = np.hstack([ -np.ones(len(self.normalTrainData[0])), np.ones(len(self.abnormalTrainData[0])) ])

        print 'Computing likelihoods'
        r = Parallel(n_jobs=-1)(delayed(learning_hmm.computeLikelihoods)(i, self.ml.A, self.ml.B, self.ml.pi, self.ml.F,
                                                                         [ trainDataX[j][i] for j in xrange(self.nEmissionDim) ],
                                                                         self.ml.nEmissionDim, self.ml.nState,
                                                                         startIdx=self.startIdx, bPosterior=True)
                                                                         for i in xrange(len(trainDataX[0])))
        _, self.ll_classifier_train_idx, ll_logp, ll_post = zip(*r)

        # nSample x nLength
        self.ll_classifier_train_X, self.ll_classifier_train_Y = learning_hmm.getHMMinducedFeatures(ll_logp, ll_post, trainDataY,
                                                                                          c=1.0, add_delta_logp=self.add_logp_d)

        # flatten the data
        print 'Flattening data'
        self.X_train_org = []
        self.Y_train_org = []
        self.idx_train_org = []
        for i in xrange(len(self.ll_classifier_train_X)):
            for j in xrange(len(self.ll_classifier_train_X[i])):
                self.X_train_org.append(self.ll_classifier_train_X[i][j])
                self.Y_train_org.append(self.ll_classifier_train_Y[i][j])
                self.idx_train_org.append(self.ll_classifier_train_idx[i][j])


    def preprocessData(self):
        # Train a scaler and data preparation
        print 'Preparing Standard Scaler and preprocessing datasets'
        self.scaler = preprocessing.StandardScaler()
        self.X_train_org = self.scaler.fit_transform(self.X_train_org)

        # scaling training data
        idx_list = range(len(self.ll_classifier_train_X))
        random.shuffle(idx_list)
        s_flag = True
        f_flag = True
        for i, count in enumerate(idx_list):
            train_X = []
            for j in xrange(len(self.ll_classifier_train_X[i])):
                train_X.append( self.scaler.transform(self.ll_classifier_train_X[i][j]) )

            if (s_flag is True and self.ll_classifier_train_Y[i][0] < 0) or True:
                s_flag = False
                self.ll_test_X.append( train_X )
                self.ll_test_Y.append( self.ll_classifier_train_Y[i] )
            elif (f_flag is True and self.ll_classifier_train_Y[i][0] > 0) or True:
                f_flag = False
                self.ll_test_X.append( train_X )
                self.ll_test_Y.append( self.ll_classifier_train_Y[i] )


    def evalClassifier(self, weight):
        print 'Evaluating the trained classifier with class weights:', weight
        self.classifier = clf.classifier(method=self.classifier_method, nPosteriors=self.nState, nLength=self.nLength - self.startIdx)
        self.classifier.set_params(class_weight=weight)
        if 'sgd' in self.classifier_method:
            self.classifier.set_params(sgd_n_iter=self.sgd_n_iter)
        self.classifier.fit(self.X_train_org, self.Y_train_org, self.idx_train_org)
        acc_all, _, _ = evaluation(list(self.ll_test_X), list(self.ll_test_Y), self.classifier)

        return acc_all


def evaluation(X, Y, clf, verbose=False):

    if X is None: return 0, 0, 0
    if len(X) is not len(Y):
        if len(np.shape(X)) == 2: X=[X]
        if len(np.shape(Y)) == 1: Y=[Y]
    if len(Y) != len(X):
        print "wrong dim: ", np.shape(X), np.shape(Y)
        sys.exit()

    tp_l = []
    fp_l = []
    fn_l = []
    tn_l = []

    if clf.method.find('svm')>=0 or clf.method.find('sgd')>=0:
        for i in xrange(len(X)):

            anomaly = False
            est_y   = clf.predict(X[i])
            for j in xrange(len(est_y)):

                if j < 4: continue
                if est_y[j] > 0:
                    anomaly = True
                    break

            if anomaly is True and  Y[i][0] > 0: tp_l += [1]
            if anomaly is True and  Y[i][0] < 0: fp_l += [1]
            if anomaly is False and  Y[i][0] > 0: fn_l += [1]
            if anomaly is False and  Y[i][0] < 0: tn_l += [1]
    else:
        print "Not available method"
        sys.exit()

    try:
        tpr = float(np.sum(tp_l)) / float(np.sum(tp_l)+np.sum(fn_l)) * 100.0
        fpr = float(np.sum(fp_l)) / float(np.sum(fp_l)+np.sum(tn_l)) * 100.0
    except:
        print "tp, fp, tn, fn: ", tp_l, fp_l, tn_l, fn_l

    if np.sum(tp_l+fn_l+fp_l+tn_l) == 0: return
    acc = float(np.sum(tp_l+tn_l)) / float(np.sum(tp_l+fn_l+fp_l+tn_l)) * 100.0
    print "tp=",np.sum(tp_l), " fn=",np.sum(fn_l), " fp=",np.sum(fp_l), " tn=",np.sum(tn_l), " ACC: ",  acc
    return acc, np.sum(fp_l), np.sum(fn_l)




if __name__ == '__main__':
    searcher = WeightSearch()

