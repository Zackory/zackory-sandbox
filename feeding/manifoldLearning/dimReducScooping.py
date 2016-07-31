from time import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import offsetbox
from sklearn import (manifold, datasets, decomposition, ensemble, discriminant_analysis, random_projection)

import os, sys, random
from sklearn import preprocessing
import hrl_lib.util as ut
import hrl_anomaly_detection.params as params
from hrl_anomaly_detection import data_manager as dm


# digits = datasets.load_digits(n_class=6)
# X = digits.data
# y = digits.target
# n_samples, n_features = X.shape
# n_neighbors = 30
# print np.shape(X), np.shape(y)


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
# X_train_org = np.delete(X_train_org, [1], axis=1)
Y_train_org = Y_train_org[samples]

# Prepare data
X = X_train_org
y = Y_train_org
n_samples, n_features = X.shape
n_neighbors = 30
n_neighbors = 20
print np.shape(X), np.shape(y)


#----------------------------------------------------------------------
# Scale and visualize the embedding vectors
def plot_embedding(X, title=None):
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


# plot_embedding(X, title='True')
# plt.show()
# sys.exit()


#----------------------------------------------------------------------
# Random 2D projection using a random unitary matrix
print("Computing random projection")
rp = random_projection.SparseRandomProjection(n_components=2, random_state=42)
X_projected = rp.fit_transform(X)
plot_embedding(X_projected, "Random Projection of the digits")


#----------------------------------------------------------------------
# Projection on to the first 2 principal components

print("Computing PCA projection")
t0 = time()
X_pca = decomposition.TruncatedSVD(n_components=2).fit_transform(X)
plot_embedding(X_pca,
               "Principal Components projection of the digits (time %.2fs)" %
               (time() - t0))

#----------------------------------------------------------------------
# Projection on to the first 2 linear discriminant components

# print("Computing Linear Discriminant Analysis projection")
# X2 = X.copy()
# X2.flat[::X.shape[1] + 1] += 0.01  # Make X invertible
# print np.shape(X2)
# t0 = time()
# X_lda = discriminant_analysis.LinearDiscriminantAnalysis(n_components=2).fit_transform(X2, y)
# print np.shape(X_lda)
# plot_embedding(X_lda,
#                "Linear Discriminant projection of the digits (time %.2fs)" %
#                (time() - t0))


#----------------------------------------------------------------------
# Isomap projection of the digits dataset
print("Computing Isomap embedding")
t0 = time()
X_iso = manifold.Isomap(n_neighbors, n_components=2).fit_transform(X)
print("Done.")
plot_embedding(X_iso,
               "Isomap projection of the digits (time %.2fs)" %
               (time() - t0))


#----------------------------------------------------------------------
# Locally linear embedding of the digits dataset
print("Computing LLE embedding")
clf = manifold.LocallyLinearEmbedding(n_neighbors, n_components=2,
                                      method='standard')
t0 = time()
X_lle = clf.fit_transform(X)
print("Done. Reconstruction error: %g" % clf.reconstruction_error_)
plot_embedding(X_lle,
               "Locally Linear Embedding of the digits (time %.2fs)" %
               (time() - t0))


#----------------------------------------------------------------------
# Modified Locally linear embedding of the digits dataset
print("Computing modified LLE embedding")
clf = manifold.LocallyLinearEmbedding(n_neighbors, n_components=2,
                                      method='modified')
t0 = time()
X_mlle = clf.fit_transform(X)
print("Done. Reconstruction error: %g" % clf.reconstruction_error_)
plot_embedding(X_mlle,
               "Modified Locally Linear Embedding of the digits (time %.2fs)" %
               (time() - t0))


#----------------------------------------------------------------------
# HLLE embedding of the digits dataset
# print("Computing Hessian LLE embedding")
# clf = manifold.LocallyLinearEmbedding(n_neighbors, n_components=2,
#                                       method='hessian')
# t0 = time()
# X_hlle = clf.fit_transform(X)
# print("Done. Reconstruction error: %g" % clf.reconstruction_error_)
# plot_embedding(X_hlle,
#                "Hessian Locally Linear Embedding of the digits (time %.2fs)" %
#                (time() - t0))


#----------------------------------------------------------------------
# LTSA embedding of the digits dataset
# print("Computing LTSA embedding")
# clf = manifold.LocallyLinearEmbedding(n_neighbors, n_components=2,
#                                       method='ltsa')
# t0 = time()
# X_ltsa = clf.fit_transform(X)
# print("Done. Reconstruction error: %g" % clf.reconstruction_error_)
# plot_embedding(X_ltsa,
#                "Local Tangent Space Alignment of the digits (time %.2fs)" %
#                (time() - t0))

#----------------------------------------------------------------------
# MDS  embedding of the digits dataset
print("Computing MDS embedding")
clf = manifold.MDS(n_components=2, n_init=1, max_iter=100)
t0 = time()
X_mds = clf.fit_transform(X)
print("Done. Stress: %f" % clf.stress_)
plot_embedding(X_mds,
               "MDS embedding of the digits (time %.2fs)" %
               (time() - t0))

#----------------------------------------------------------------------
# Random Trees embedding of the digits dataset
print("Computing Totally Random Trees embedding")
hasher = ensemble.RandomTreesEmbedding(n_estimators=200, random_state=0,
                                       max_depth=5)
t0 = time()
X_transformed = hasher.fit_transform(X)
pca = decomposition.TruncatedSVD(n_components=2)
X_reduced = pca.fit_transform(X_transformed)

plot_embedding(X_reduced,
               "Random forest embedding of the digits (time %.2fs)" %
               (time() - t0))

#----------------------------------------------------------------------
# Spectral embedding of the digits dataset
print("Computing Spectral embedding")
embedder = manifold.SpectralEmbedding(n_components=2, random_state=0,
                                      eigen_solver="arpack")
t0 = time()
X_se = embedder.fit_transform(X)

plot_embedding(X_se,
               "Spectral embedding of the digits (time %.2fs)" %
               (time() - t0))

#----------------------------------------------------------------------
# t-SNE embedding of the digits dataset
print("Computing t-SNE embedding")
tsne = manifold.TSNE(n_components=2, init='pca', random_state=0)
t0 = time()
X_tsne = tsne.fit_transform(X)

plot_embedding(X_tsne,
               "t-SNE embedding of the digits (time %.2fs)" %
               (time() - t0))

plt.show()
