init:
	bash ./scripts/init.sh

test:
	bash ./scripts/test.sh

run:
	bash ./scripts/run.sh

clean:
	rm logs/*

test_tracker:
	bash ./scripts/test_tracker.sh

test_controls:
	bash ./scripts/test_controls.sh

.PHONY: init test run clean test_tracker test_controls
