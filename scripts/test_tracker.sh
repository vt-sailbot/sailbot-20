#!/bin/bash

if [[ $HOSTNAME == beaglebone ]]; then
	python3.5 integration_test.py tracker
else
	. ./p3_5env/bin/activate; \
	python integration_test.py tracker
fi
