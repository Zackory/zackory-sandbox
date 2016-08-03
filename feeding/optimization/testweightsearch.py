import os, sys, rospy
import numpy as np
import cPickle as pickle
from collections import deque
from joblib import Parallel, delayed
from sklearn.grid_search import GridSearchCV

import hrl_lib.util as ut
from hrl_anomaly_detection import util
from hrl_anomaly_detection import data_manager as dm
from hrl_anomaly_detection.ICRA2017_params import *

# learning
from hrl_anomaly_detection.hmm import learning_hmm
from hrl_anomaly_detection.hmm import learning_util as hmm_util

# Classifier
from hrl_anomaly_detection.classifiers import classifier as clf
from hrl_anomaly_detection.classifiers.classifier_util import *

import weightsearch

class TestWeightSearch:
    def __init__(self):
        self.initParams()
        # Load HMM and Training/Test Data
        self.loadAll()


    def initParams(self):
        print 'Loading and initializing all parameters'
        self.hmm_renew = False
        self.sgd_n_iter = 100
        self.startIdx  = 4
        self.nonAnomalousX = []
        self.nonAnomalousY = []
        self.anomalousX = []
        self.anomalousY = []

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
            ret = self.ml.fit(self.normalTrainData, cov_mult=[self.cov]*(self.nEmissionDim**2))

        if ret == 'Failure':
            print '-'*25, '\nHMM training failed.\n', '-'*25
            sys.exit()


    def prepareClassifierData(self):
        print 'Preparing data for classifier'
        trainDataX = [np.vstack([self.normalTrainData[i], self.abnormalTrainData[i]]) for i in xrange(self.nEmissionDim)]
        trainDataY = np.hstack([-np.ones(len(self.normalTrainData[0])), np.ones(len(self.abnormalTrainData[0]))])

        print 'Computing likelihoods'
        r = Parallel(n_jobs=-1)(delayed(learning_hmm.computeLikelihoods)(i, self.ml.A, self.ml.B, self.ml.pi, self.ml.F,
                                                                         [ trainDataX[j][i] for j in xrange(self.nEmissionDim) ],
                                                                         self.ml.nEmissionDim, self.ml.nState,
                                                                         startIdx=self.startIdx, bPosterior=True)
                                                                         for i in xrange(len(trainDataX[0])))
        _, self.ll_classifier_train_idx, ll_logp, ll_post = zip(*r)

        # Get features from log and posterior (nSample x nLength)
        self.ll_classifier_train_X, self.ll_classifier_train_Y = learning_hmm.getHMMinducedFeatures(ll_logp, ll_post, trainDataY,
                                                                                          c=1.0, add_delta_logp=self.add_logp_d)

        self.X_train_org = self.ll_classifier_train_X
        self.Y_train_org = self.ll_classifier_train_Y
        self.idx_train_org = self.ll_classifier_train_idx


    def evaluation(self, clf, X, Y, verbose=False):
        # Reshape X and Y
        # print 'eval', np.shape(X), np.shape(Y)
        # X = np.reshape(X, (np.shape(X)[0] / 195, 195, np.shape(X)[-1]))
        # Y = np.reshape(Y, (np.shape(Y)[0] / 195, 195))

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
        return acc, np.sum(fp_l), np.sum(fn_l)


    def performWeightSearch(self):
        classifier = clf.classifier(method=self.classifier_method, nPosteriors=self.nState, nLength=self.nLength - self.startIdx)
        if 'sgd' in self.classifier_method:
            classifier.set_params(sgd_n_iter=self.sgd_n_iter)
        ws = weightsearch.WeightSearch(classifier, self.evaluation)
        weights = [0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.5, 0.75, 1.0, 2.0, 4.0, 6.0, 8.0, 10.0]
        bestWeight, bestAccuracy = ws.findBestWeight(weights, self.X_train_org, self.Y_train_org)
        print 'Best weight found:', bestWeight, 'with accuracy:', bestAccuracy


if __name__ == '__main__':
    tws = TestWeightSearch()
    tws.performWeightSearch()

