import numpy as np
import matplotlib.pyplot as plt
from sklearn.manifold import Isomap

import os, sys, random, time
from sklearn import preprocessing
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

# Flatten data
X_train_org, Y_train_org, idx_train_org = dm.flattenSample(ll_classifier_train_X, ll_classifier_train_Y, ll_classifier_train_idx, remove_fp=True)
Xtestflat, Ytest, _ = [np.array(x) for x in dm.flattenSample(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)]
print np.shape(X_train_org), np.shape(Y_train_org), np.shape(ll_classifier_test_X), np.shape(ll_classifier_train_Y)

# Scale data
# scaler = preprocessing.StandardScaler()
# X_train_org = scaler.fit_transform(X_train_org)
# X_train_org = preprocessing.normalize(X_train_org, norm='l2')

X_train_org = np.array(X_train_org)
Y_train_org = np.array(Y_train_org)
samples = random.sample(range(len(X_train_org)), 1000)
X_train_org = X_train_org[samples]
Y_train_org = Y_train_org[samples]

Xtestflat = np.array(Xtestflat)
Ytest = np.array(Ytest)
samples = random.sample(range(len(Xtestflat)), 1000)
Xtestflat = Xtestflat[samples]
Ytest = Ytest[samples]

# Prepare data
X = X_train_org
y = Y_train_org
print np.shape(X), np.shape(y)



neighbors = 20
iso = Isomap(neighbors, n_components=2)
Xiso = iso.fit_transform(X)
Xtestiso = iso.transform(Xtestflat)

print np.shape(Xiso), np.shape(X)


# Scale training data for SVM
scaler = preprocessing.StandardScaler()
Xtrain = Xiso
Xtest = Xtestiso
#Xtrain = np.array(scaler.fit_transform(Xiso))
#Xtest = np.array(scaler.transform(Xtestiso))

weights = ROC_dict['svm_param_range']
clf = cf.classifier(method='svm', nPosteriors=nState, nLength=nLength)
# clf.set_params(class_weight=weights[-1])
clf.fit(Xtrain, Y_train_org, idx_train_org, parallel=False)

print 'Score:', np.mean([1 if y == z else 0 for y, z in zip(Ytest, clf.predict(Xtestiso))])



