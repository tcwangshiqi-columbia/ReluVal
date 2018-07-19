'''Directly running this python script will train the
fully connected DNN for MNIST dataset.
This script can also be loaded as the pretrained model
by others
'''

from __future__ import print_function

import keras
from keras.datasets import mnist
from keras.layers import Dense, Input, Activation
from keras.models import Model
from keras.optimizers import RMSprop
import numpy as np
from configs import bcolors


def FCModel(train=False):
    if train:
        batch_size = 128
        num_classes = 10
        epochs = 20

        # the data, shuffled and split between train and test sets
        (x_train, y_train), (x_test, y_test) = mnist.load_data()

        x_train = x_train.reshape(60000, 784)
        x_test = x_test.reshape(10000, 784)
        x_train = x_train.astype('float32')
        x_test = x_test.astype('float32')
        x_train /= 255
        x_test /= 255

        # convert class vectors to binary class matrices
        y_train = keras.utils.to_categorical(y_train, num_classes)
        y_test = keras.utils.to_categorical(y_test, num_classes)

    # define architecture
    in_placeholder = Input(shape=(784,))
    x = Dense(512, activation='relu')(in_placeholder)
    x = Dense(512, activation='relu')(x)
    x = Dense(10, name='before_softmax')(x)
    x = Activation('softmax', name='predictions')(x)
    model = Model(in_placeholder, x)

    if train:
        model.compile(loss='categorical_crossentropy',
                      optimizer=RMSprop(),
                      metrics=['accuracy'])
        model.fit(x_train, y_train,
                  batch_size=batch_size,
                  epochs=epochs,
                  verbose=1,
                  validation_data=(x_test, y_test))

        print(bcolors.OKBLUE + 'FC model trained' + bcolors.ENDC)
        score = model.evaluate(x_test, y_test, verbose=0)
        print(bcolors.OKGREEN + 'Test loss: {}'.format(score[0]) + bcolors.ENDC)
        print(bcolors.OKGREEN + 'Test Accuracy: {}'.format(score[1]) + bcolors.ENDC)
        model.save_weights('./FCModel.h5')
    else:
        model.load_weights('./FCModel.h5')
        print(bcolors.OKBLUE + 'FC model loaded' + bcolors.ENDC)

    return model


if __name__ == '__main__':
    model = FCModel(False)
    (_, _), (x_test, y_test) = mnist.load_data()
    x_test = x_test.reshape(10000, 784)
    x_test = x_test.astype('float32')
    x = x_test[0:1]
    x /= 255
    print(x.shape)
    #predict = np.argmax(model.predict(x_test), axis = 1)
    intermediate_layer_model = Model(inputs=model.input,
                                 outputs=model.get_layer('before_softmax').output)
    
    predicts = intermediate_layer_model.predict(x)
    print(predicts)
    '''
    for i in range(x.shape[0]):
        print(str(x[i])+",", end="")
    '''
    # x_test /= 255
    # print(x_test.shape)
    # predict = np.argmax(model.predict(x_test), axis = 1)
    # print(1-np.count_nonzero(predict - y_test)/10000.0)
    '''
    weights = [model.get_weights()[0].T, model.get_weights()[2].T, model.get_weights()[4].T]
    bias = [model.get_weights()[1], model.get_weights()[3], model.get_weights()[5]]
    layer_Num = 3;

    for layer in range(layer_Num):
        for i in range(weights[layer].shape[0]):
            for j in range(weights[layer].shape[1]):
                print(str(weights[layer][i,j])+',', end = "")
            print("")
        for i in range(bias[layer].shape[0]):
            print(bias[layer][i], end="")
            print(",")
    '''