#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


#include "subscriber.h"
#include "relay.h"
#include "msg_types.h"
#include "circular_buffer.h"


// Functions

Subscriber* subscribe(Relay* relay, char* channel_name, PyObject* callback) {
    Subscriber* new_sub = malloc(sizeof(Subscriber));

    new_sub->id = (char*)calloc((strlen(channel_name) + 6), sizeof(char));
    sprintf(new_sub->id, "%s_%5f", channel_name, (double)clock());

    new_sub->py_callback = callback;

    register_subscriber_on_channel(relay, channel_name, new_sub);

    return new_sub;
}


void* data_callback(void* callback_with_data) {
    /*
     * See https://docs.python.org/3/extending/extending.html for CPython API documentation.
     */

    Data* data = ((CallbackWithData*)callback_with_data)->data;
    PyObject* py_callback = ((CallbackWithData*)callback_with_data)->py_callback;

    // Adds the Python function to the Python ref counter
    Py_XINCREF(py_callback);

    PyObject *result;
    PyObject *arglist;

    // Unpickles the local copy of the object passed by publisher
    arglist = Py_BuildValue("(s,i)", data->data, data->size);

    // Calls the Python function
    result = PyEval_CallObject(py_callback, arglist);

//    free(subscriber_data);

    // Have to return something
    return result;
}

void unsubscribe(Relay* relay, Subscriber **subscriber) {
    // TODO: Remove from channel

    free((**subscriber).py_callback);
    free(*subscriber);
    *subscriber = (Subscriber*)NULL;
}
