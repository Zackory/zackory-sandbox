import numpy as np
from sklearn import preprocessing, cross_validation

class WeightSearch:
    def __init__(self, classifier, evalFunction):
        self.classifier = classifier
        self.evalFunction = evalFunction

    def findBestWeight(self, weightsList, unflattenedX, unflattenedY, itersPerWeight=10):
        '''
        Returns the best weight from a provided weights list given a specific classifier, evaluation function, and unflattened data
        '''
        unflattenedX = np.array(unflattenedX)
        unflattenedY = np.array(unflattenedY)
        # Determine which data are anomalous and nonanomalous
        nonAnomalousX = []
        nonAnomalousY = []
        anomalousX = []
        anomalousY = []
        for X, y in zip(unflattenedX, unflattenedY):
            if y[0] < 0:
                nonAnomalousX.append(X)
                nonAnomalousY.append(y)
            else:
                anomalousX.append(X)
                anomalousY.append(y)

        # Flatten data and train a Standard Scaler
        flatX = np.reshape(unflattenedX, (-1, np.shape(unflattenedX)[-1]))
        scaler = preprocessing.StandardScaler()
        scaler.fit(flatX)

        weightAccuracy = []
        # Try every weight
        for weight in weightsList:
            # Set class weight for classifier
            self.classifier.set_params(class_weight=weight)
            # Create itersPerWeight randomized splits while maintaining an even balance of anomalous and nonanomalous data in each training set and validation set
            split = cross_validation.StratifiedShuffleSplit([1]*len(nonAnomalousX) + [-1]*len(anomalousX), n_iter=itersPerWeight, test_size=0.4)
            accuracies = []
            for trainIdx, valIdx in split:
                # Set up training and validation splits
                trainX, trainY = unflattenedX[trainIdx], unflattenedY[trainIdx]
                valX, valY = unflattenedX[valIdx], unflattenedY[valIdx]

                # Flatten and scale data, then fit classifier
                trainX = np.reshape(trainX, (-1, np.shape(trainX)[-1]))
                trainY = np.reshape(trainY, -1)
                self.classifier.fit(scaler.transform(trainX), trainY)
                # Scale validation data and calculate accuracy
                for i in xrange(len(valX)):
                    valX[i] = scaler.transform(valX[i])
                results = self.evalFunction(self.classifier, valX, valY)
                if type(results) is list or type(results) is tuple:
                    accuracies.append(results[0])
                else:
                    accuracies.append(results)
            weightAccuracy.append(np.mean(accuracies))
            print 'Mean accuracy for weight %f:' % weight, np.mean(accuracies)
        bestWeightIndex = np.argmax(weightAccuracy)
        bestWeight = weightsList[bestWeightIndex]

        return bestWeight, weightAccuracy[bestWeightIndex]

