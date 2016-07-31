import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
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
modelingPkl = '/home/zackory/hrl_file_server/dpark_data/anomaly/RSS2016/old/scooping_data/200_3/hmm_scooping_0.pkl'
print modelingPkl
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
Xtrainflat, Ytrain, idxtrain = dm.flattenSample(ll_classifier_train_X, ll_classifier_train_Y, ll_classifier_train_idx, remove_fp=True)

# Scale training data for SVM
scaler = preprocessing.StandardScaler()
Xtrain = np.array(scaler.fit_transform(Xtrainflat))

# Train classifier
weights = ROC_dict['svm_param_range']
clf = cf.classifier(method='svm', nPosteriors=nState, nLength=nLength)
clf.set_params(**SVM_dict)
clf.set_params(class_weight=weights[-1])
clf.fit(Xtrain, Ytrain, idxtrain, parallel=False)

nonanomalous = [0] * (len(ll_classifier_test_X) / 10 + 1)
anomalous = [0] * (len(ll_classifier_test_X) / 10 + 1)
print len(anomalous)

# Determine which sequences are classified as anomalous
print np.shape(ll_classifier_test_X), np.shape(ll_classifier_test_Y)
for i, (Xval, yval, idxval) in enumerate(zip(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)):
    # Flatten data
    Xvalflat, yvalflat, _ = dm.flattenSample(np.array([Xval]), np.array([yval]), np.array([idxval]))
    print np.shape(Xvalflat), np.shape(yvalflat), i / 10

    # Scale validation data for SVM
    Xvalflat = np.array(scaler.transform(Xvalflat))

    # Predict test data
    Z = np.array(clf.predict(Xvalflat))

    if any(Z == 1):
        print 'Abnormal point'
        # print Z == 1
        # anomalous.append((i / 10) * 10)
        anomalous[i / 10] += 1
    else:
        # nonanomalous.append((i / 10) * 10)
        nonanomalous[i / 10] += 1

# Change color scheme
# colormap = plt.cm.gist_ncar
# mpl.rcParams['axes.color_cycle'] = [colormap(i) for i in np.linspace(0, 0.9, 2)]

# Plot histogram
indices = xrange(0, len(nonanomalous)*10, 10)
barwidth = 3

# plt.subplot(2, 1, 1)
plt.figure()
plt.bar(np.array(indices) - barwidth, nonanomalous, width=barwidth)
plt.bar(indices, anomalous, width=barwidth, color='green')
plt.title('Last 10 Observations')
plt.xlabel('Number of new observation sequences')
plt.ylabel('Number of observation sequences')
plt.legend(['nonanomalous', 'anomalous'])
plt.grid(True)
#plt.xlim([-2, 30])
plt.ylim([0, np.max([nonanomalous, anomalous]) + 5])
plt.xticks(indices)

# Create cumulative data
for i in xrange(1, len(nonanomalous)):
    nonanomalous[i] += nonanomalous[i - 1]
    anomalous[i] += anomalous[i - 1]

# plt.subplot(2, 1, 2)
plt.figure()
plt.bar(np.array(indices) - barwidth, nonanomalous, width=barwidth)
plt.bar(indices, anomalous, width=barwidth, color='green')
plt.title('Cumulative Observations')
plt.xlabel('Number of new observation sequences')
plt.ylabel('Number of total observation sequences')
plt.legend(['nonanomalous', 'anomalous'])
plt.grid(True)
#plt.xlim([-2, 30])
plt.ylim([0, np.max([nonanomalous, anomalous]) + 5])
plt.xticks(indices)

plt.show()

