import numpy as np
import matplotlib.pyplot as plt
from matplotlib import offsetbox
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis

import os, sys, random, time
from sklearn import preprocessing
import hrl_lib.util as ut
import hrl_anomaly_detection.params as params
from hrl_anomaly_detection import data_manager as dm


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
print np.shape(X_train_org), np.shape(Y_train_org), np.shape(ll_classifier_test_X), np.shape(ll_classifier_train_Y)

# Scale data
# scaler = preprocessing.StandardScaler()
# X_train_org = scaler.fit_transform(X_train_org)
# X_train_org = preprocessing.normalize(X_train_org, norm='l2')

X_train_org = np.array(X_train_org)
Y_train_org = np.array(Y_train_org)
# X_train_org = X_train_org[:3000]
samples = random.sample(range(len(X_train_org)), 1000)
X_train_org = X_train_org[samples] # [:, 2:]
Y_train_org = Y_train_org[samples]
X = np.delete(X_train_org, [1], axis=1)

# Prepare data
# X = X_train_org
y = Y_train_org
print np.shape(X), np.shape(y)


def plot_embedding(X, title=None, scale=False):
    if scale:
        x_min, x_max = np.min(X, 0), np.max(X, 0)
        X = (X - x_min) / (x_max - x_min)

    plt.figure()
    ax = plt.subplot(111)
    # Create the colormap for the training points
    YtrainColors = np.array(['green' if yy == 1 else 'red' for yy in y])
    plt.scatter(X_train_org[:, 1], X, c=YtrainColors)

    # plt.xticks([]), plt.yticks([])
    if title is not None:
        plt.title(title)

print("Computing Linear Discriminant Analysis projection")
# X2 = X.copy()
# X2.flat[::X.shape[1] + 1] += 0.01  # Make X invertible
# print np.shape(X2)
t0 = time.time()
X_lda = LinearDiscriminantAnalysis(n_components=2).fit_transform(X, y)
print np.shape(X_lda)
plot_embedding(X_lda, "Linear Discriminant projection of the digits (time %.2fs)" % (time.time() - t0))

# neighbors = 20
# iso = Isomap(neighbors, n_components=2)
# isomap(iso, X, title='Standard isomap')
# isomap(iso, preprocessing.scale(X), title='Scaled isomap')
# isomap(iso, preprocessing.normalize(X, norm='l2'), title='l2 norm isomap')

plt.show()
