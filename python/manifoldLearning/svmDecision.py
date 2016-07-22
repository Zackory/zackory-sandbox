
import numpy as np
import matplotlib.pyplot as plt
import os, sys, random, time
from sklearn import preprocessing, svm

import hrl_lib.util as ut
import hrl_anomaly_detection.params as params
from hrl_anomaly_detection import data_manager as dm
import hrl_anomaly_detection.classifiers.classifier as cf




# Load yogurt scooping data
subjects = ['Wonyoung', 'Tom', 'lin', 'Ashwin', 'Song', 'Henry2']
_, saveDataPath, paramDict = params.getScooping(task='scooping', data_renew=False, AE_renew=False, HMM_renew=False, rf_center='kinEEPos', local_range=10.0, ae_swtch=False, dim=3)
ROC_dict = paramDict['ROC']
SVM_dict = paramDict['SVM']
nPoints = ROC_dict['nPoints']

modelingPkl = os.path.join(saveDataPath, 'hmm_scooping_0.pkl')
d = ut.load_pickle(modelingPkl)
nState = d['nState']
ll_classifier_train_X   = d['ll_classifier_train_X']
ll_classifier_train_Y   = d['ll_classifier_train_Y']
ll_classifier_train_idx = d['ll_classifier_train_idx']
ll_classifier_test_X    = d['ll_classifier_test_X']
ll_classifier_test_Y    = d['ll_classifier_test_Y']
ll_classifier_test_idx  = d['ll_classifier_test_idx']
nLength = d['nLength']

print nState, nLength

# Flatten data
Xtrainflat, Y_train_org, idx_train_org = dm.flattenSample(ll_classifier_train_X, ll_classifier_train_Y, ll_classifier_train_idx, remove_fp=True)
Xtestflat, Ytest, _ = [np.array(x) for x in dm.flattenSample(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)]

Xtrainflat = np.array(Xtrainflat)

# Setup classifier testing data
x_min, x_max = Xtrainflat[:, 0].min(), Xtrainflat[:, 0].max() # Min/Max for log-likelihood
z_min, z_max = Xtrainflat[:, 1].min(), Xtrainflat[:, 1].max() # Min/Max for log-likelihood
# xx, yy = np.meshgrid(np.linspace(x_min, x_max, 500), np.linspace(y_min, y_max, 500))
xx, yy = np.meshgrid(xrange(len(ll_classifier_train_X[0])), np.linspace(x_min, x_max, 500))
_, zz = np.meshgrid(xrange(len(ll_classifier_train_X[0])), np.linspace(z_min, z_max, 500))

postExp = []
for i in xrange(len(ll_classifier_train_X[0])):
    post = np.array(ll_classifier_train_X)[:, i, 2:]
    post = np.mean(post, axis=0)
    post /= np.sum(post)
    postExp.append(post)
postExp = np.array(postExp)
Xtest = np.hstack([np.array([yy.ravel()]).T, np.array([zz.ravel()]).T, postExp[xx.ravel().tolist()]])

# Scale training data for SVM
scaler = preprocessing.StandardScaler()
Xtrain = np.array(scaler.fit_transform(Xtrainflat))
Xtestflat = np.array(scaler.transform(Xtestflat))
Xtest = np.array(scaler.transform(Xtest))

print np.shape(Xtrain), np.shape(Xtestflat)


weights = ROC_dict['svm_param_range']
print 'Using weights:', weights[-1]
clf = cf.classifier(method='svm', nPosteriors=nState, nLength=nLength)
# clf.set_params(**SVM_dict)
clf.set_params(class_weight=weights[-1])
clf.fit(Xtrain, Y_train_org, idx_train_org, parallel=False)

print 'Score:', np.mean([1 if y == z else 0 for y, z in zip(Ytest, clf.predict(Xtestflat))])

Z = np.array(clf.decision_function(Xtest))
print Z.min(), Z.max()



