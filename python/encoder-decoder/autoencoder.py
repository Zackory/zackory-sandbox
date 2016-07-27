import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from sklearn import preprocessing
from scipy.stats import entropy

from keras.layers import Input, Dense
from keras.models import Model

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

saveDataPath = '/home/zackory/hrl_file_server/dpark_data/anomaly/RSS2016/old/scooping_data/200_3'
modelingPkl = os.path.join(saveDataPath, 'hmm_scooping_0.pkl')
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
Xvalflat, yval, _ = dm.flattenSample(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)

# Scale training data for SVM
scaler = preprocessing.StandardScaler()
Xtrain = np.array(scaler.fit_transform(Xtrainflat))
Xval = np.array(scaler.transform(Xvalflat))
# Xtrain = Xtrainflat
# Xval = Xvalflat


def simpleAutoencoder():
    global encoder, decoder, autoencoder
    inputDim = 27
    encodingDim = 2

    inputVec = Input(shape=(inputDim,))
    # Encoded representation of the input
    encoded = Dense(encodingDim, activation='relu')(inputVec)
    # Lossy reconstruction of the input
    decoded = Dense(inputDim, activation='sigmoid')(encoded)

    # This model maps an input to its reconstruction
    autoencoder = Model(input=inputVec, output=decoded)

    # This model maps an input to its encoded representation
    encoder = Model(input=inputVec, output=encoded)

    # This model maps an encoded representation to its reconstruction
    encodedInput = Input(shape=(encodingDim,))
    decoderLayer = autoencoder.layers[-1]
    # Create the decoder model
    decoder = Model(input=encodedInput, output=decoderLayer(encodedInput))

    # autoencoder.compile(optimizer='nadam', loss='mse')
    autoencoder.compile(optimizer='rmsprop', loss='mse')

def deepAutoencoder():
    global encoder, decoder, autoencoder
    inputDim = 27
    encodingDim = 2

    inputVec = Input(shape=(inputDim,))
    encoded = Dense(20, activation='relu')(inputVec)
    encoded = Dense(10, activation='relu')(encoded)
    encoded = Dense(encodingDim, activation='relu')(encoded)

    decoded = Dense(10, activation='relu')(encoded)
    decoded = Dense(20, activation='relu')(decoded)
    decoded = Dense(inputDim, activation='sigmoid')(decoded)

    autoencoder = Model(input=inputVec, output=decoded)

    encoder = Model(input=inputVec, output=encoded)

    encodedInput = Input(shape=(encodingDim,))
    decoderLayer = autoencoder.layers[-1](autoencoder.layers[-2](autoencoder.layers[-3](encodedInput)))
    decoder = Model(input=encodedInput, output=decoderLayer)

    autoencoder.compile(optimizer='rmsprop', loss='mse')



# simpleAutoencoder()
# deepAutoencoder()


autoencoder.fit(Xtrain, Xtrain, nb_epoch=100, batch_size=64, shuffle=True, validation_data=(Xval, Xval))

# Encode and decode some validation vectors
encodedVecs = encoder.predict(Xval)
decodedVecs = decoder.predict(encodedVecs)
absError = []
for xx, yy in zip(Xval, decodedVecs):
    absError.append(np.abs(xx - yy))
print 'Encoded shape:', np.shape(encodedVecs), 'Decoded shape:', np.shape(decodedVecs), 'Abs error shape:', np.shape(absError)
print 'Mean abs errors:', np.mean(absError, axis=1)


# Display the first 10 encoding / decoding pairs
for i in xrange(1):
    print 'True:', Xval[i]
    print 'Decoded:', decodedVecs[i]
    print 'Encoded:', encodedVecs[i]


# Visualize the encoded 2D vectors
plt.figure()
YtrainColors = np.array(['green' if yy == 1 else 'red' for yy in yval])
plt.scatter(encodedVecs[:, 0], encodedVecs[:, 1], c=YtrainColors)
plt.show()

