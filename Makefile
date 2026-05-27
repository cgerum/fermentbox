all: frontend backend

.PHONY: api-contract
api-contract:
	cd fermentbox-frontend && yarn install --ignore-engines
	cd fermentbox-frontend && yarn api:check
	cd fermentbox-frontend && yarn api:verify-fresh

.PHONY: test-frontend
test-frontend:
	cd fermentbox-frontend && yarn install --ignore-engines
	cd fermentbox-frontend && yarn api:check
	cd fermentbox-frontend && yarn api:verify-fresh
	cd fermentbox-frontend && yarn test

.PHONY: test-backend-api-host
test-backend-api-host:
	node scripts/test-backend-api-host.mjs

.PHONY: test-backend-unit
test-backend-unit:
	mkdir -p /tmp/fermentbox-backend-unit-tests
	c++ -std=c++17 -Wall -Wextra -pedantic -DFERMENTBOX_EXPERIMENTAL_RL_CONTROLLER=1 -Ifermentbox-backend/include fermentbox-backend/tests/nn_controller_test.cpp fermentbox-backend/app/nn_controller.cpp -o /tmp/fermentbox-backend-unit-tests/nn_controller_test
	/tmp/fermentbox-backend-unit-tests/nn_controller_test

.PHONY: test-backend-browser-e2e
test-backend-browser-e2e:
	node scripts/test-backend-browser-e2e.mjs

.PHONY: start-full-app-host
start-full-app-host:
	node scripts/start-full-app-host.mjs

.PHONY: frontend
frontend:
	cd fermentbox-frontend && yarn install --ignore-engines
	cd fermentbox-frontend && yarn api:check
	cd fermentbox-frontend && yarn api:verify-fresh
	cd fermentbox-frontend && yarn build
	rm -rf fermentbox-backend/files
	mkdir -p fermentbox-backend/files
	cp -r fermentbox-frontend/dist/* fermentbox-backend/files
	find fermentbox-backend/files -name '*.map' -exec rm '{}' \;
	find fermentbox-backend/files -name '*.js' -exec gzip -f '{}' \;
	find fermentbox-backend/files -name '*.css' -exec gzip -f '{}' \;
	mv fermentbox-backend/files/js/* fermentbox-backend/files
	mv fermentbox-backend/files/css/* fermentbox-backend/files

	sed -i s#href=/css/#href=/#g fermentbox-backend/files/index.html
	sed -i s#href=/js/#href=/#g fermentbox-backend/files/index.html
	sed -i s#src=/js/#src=/#g fermentbox-backend/files/index.html

.PHONY: backend
backend:
	make -C fermentbox-backend buildpart
	make -C fermentbox-backend rebuild


.PHONY: flash
flash: all
	make -C fermentbox-backend flash

.PHONY: clean
clean:
	make -C fermentbox-backend clean
