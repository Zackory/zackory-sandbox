import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from sklearn import preprocessing
from scipy.stats import entropy

from keras.models import Model, Sequential
from keras.layers import Input, Dense, LSTM, RepeatVector

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
# Xvalflat, yval, _ = dm.flattenSample(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)

# Scale training data for SVM
scaler = preprocessing.StandardScaler()
Xtrainflat = np.array(scaler.fit_transform(Xtrainflat))
# Xval = np.array(scaler.transform(Xvalflat))

# Train classifier
weights = ROC_dict['svm_param_range']
clf = cf.classifier(method='svm', nPosteriors=nState, nLength=nLength)
clf.set_params(**SVM_dict)
clf.set_params(class_weight=weights[-1])
clf.fit(Xtrainflat, Ytrain, idxtrain, parallel=False)

graphColors = []
for i, (Xval, yval, idxval) in enumerate(zip(ll_classifier_test_X, ll_classifier_test_Y, ll_classifier_test_idx)):
    # Flatten data
    Xvalflat, yvalflat, _ = dm.flattenSample(np.array([Xval]), np.array([yval]), np.array([idxval]))
    print np.shape(Xvalflat), np.shape(yvalflat), i / 10
    # Scale validation data for SVM
    Xvalflat = np.array(scaler.transform(Xvalflat))
    # Predict test data
    Z = np.array(clf.predict(Xvalflat))
    if any(Z == 1):
        graphColors.append('red')
    else:
        graphColors.append('green')


Xtrain = ll_classifier_train_X
Xval = ll_classifier_test_X
print 'Shapes:', np.shape(Xtrain), np.shape(Xval)


def simpleAutoencoder(timesteps=195, inputDim=27, encodingDim=2):
    global encoder, decoder, autoencoder

    inputVec = Input(shape=(inputDim,))
    encoded = LSTM(encodingDim)(inputVec)
    decoded = LSTM(inputDim)(encoded)

    autoencoder = Model(input=inputVec, output=decoded)

    encoder = Model(input=inputVec, output=encoded)

    encodedInput = Input(shape=(encodingDim,))
    decoderLayer = autoencoder.layers[-1]
    decoder = Model(input=encodedInput, output=decoderLayer(encodedInput))

    autoencoder.compile(optimizer='rmsprop', loss='mse')

# encodedInput = Input(shape=(encodingDim,))
# decoderLayer = autoencoder.layers[-1](autoencoder.layers[-2](autoencoder.layers[-3](encodedInput)))
# decoder = Model(input=encodedInput, output=decoderLayer)

def lstmAutoencoder(timesteps=195, inputDim=27, encodingDim=2):
    global encoder, decoder, autoencoder

    inputs = Input(shape=(timesteps, inputDim))
    encoded = LSTM(encodingDim, return_sequences=True)(inputs)

    # decoded = RepeatVector(timesteps)(encoded)
    decoded = LSTM(inputDim, return_sequences=True)(encoded)

    autoencoder = Model(inputs, decoded)
    encoder = Model(inputs, encoded)

    autoencoder.compile(optimizer='rmsprop', loss='mse')

def encodeco(timesteps=195, featureCount=27, encodingDim=2):
    # model = Sequential()
    # model.add(LSTM(encodingDim, input_shape=(timesteps, featureCount), activation='tanh', return_sequences=True))
    # model.add(LSTM(featureCount, input_shape=(timesteps, featureCount), activation='tanh', return_sequences=True))

    inputs = Input(shape=(timesteps, featureCount))
    encoded = LSTM(encodingDim, activation='tanh', return_sequences=True)(inputs)
    decoded = LSTM(featureCount, activation='tanh', return_sequences=True)(encoded)

    model = Model(inputs, decoded)
    encoder = Model(inputs, encoded)
    decodeInputs = Input(shape=(timesteps, encodingDim))
    decoder = Model(decodeInputs, model.layers[-1](decodeInputs))

    model.compile(loss='mse', optimizer='rmsprop', metrics=['accuracy'])
    return model, encoder, decoder

# lstmAutoencoder()
# simpleAutoencoder()
autoencoder, encoder, decoder = encodeco()


autoencoder.fit(Xtrain, Xtrain, nb_epoch=5, batch_size=2, shuffle=True, validation_data=(Xval, Xval))

# Encode and decode some validation vectors
encodedVecs = encoder.predict(Xval)
decodedVecs = decoder.predict(encodedVecs)

print encodedVecs.shape

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

