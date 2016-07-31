import numpy as np
import matplotlib.pyplot as plt
from sklearn.manifold import Isomap

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
# X_train_org = np.delete(X_train_org, [1], axis=1)

# Prepare data
X = X_train_org
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
    plt.scatter(X[:, 0], X[:, 1], c=YtrainColors)

    # plt.xticks([]), plt.yticks([])
    if title is not None:
        plt.title(title)

def isomap(iso, X, title='Isomap projection', scale=False):
    print 'Computing Isomap embedding'
    t = time.time()
    Xiso = iso.fit_transform(X)
    plot_embedding(Xiso, title + '(time %.2fs)' % (time.time() - t), scale=scale)

neighbors = 20
iso = Isomap(neighbors, n_components=2)
isomap(iso, X, title='Standard isomap')
isomap(iso, preprocessing.scale(X), title='Scaled isomap')
isomap(iso, preprocessing.normalize(X, norm='l2'), title='l2 norm isomap')

# params = iso.kernel_pca_.get_params()
# params['kernel'] = 'rbf'
# params['gamma'] = SVM_dict['gamma']
# iso.kernel_pca_.set_params(**params)
# isomap(iso, X, title='RBF kernel isomap')
# print iso.kernel_pca_.get_params()

# iso = Isomap(30, n_components=2)
# isomap(iso, X, title='Standard isomap 30 neighbors')
# iso = Isomap(10, n_components=2)
# isomap(iso, X, title='Standard isomap 10 neighbors')

# iso = Isomap(neighbors, n_components=2, eigen_solver='dense')
# isomap(iso, X, title='Dense solver isomap')
# iso = Isomap(neighbors, n_components=2, eigen_solver='arpack')
# isomap(iso, X, title='Arpack solver isomap')

# iso = Isomap(neighbors, n_components=2, neighbors_algorithm='brute')
# isomap(iso, X, title='Brute algorithm isomap')
# iso = Isomap(neighbors, n_components=2, neighbors_algorithm='kd_tree')
# isomap(iso, X, title='Kd tree algorithm isomap')
# iso = Isomap(neighbors, n_components=2, neighbors_algorithm='ball_tree')
# isomap(iso, X, title='Ball tree algorithm isomap')


#----------------------------------------------------------------------
# MDS  embedding of the digits dataset
# print("Computing MDS embedding")
# clf = manifold.MDS(n_components=2, n_init=1, max_iter=100)
# t0 = time()
# X_mds = clf.fit_transform(X)
# print("Done. Stress: %f" % clf.stress_)
# plot_embedding(X_mds,
#                "MDS embedding of the digits (time %.2fs)" %
#                (time() - t0))

plt.show()
