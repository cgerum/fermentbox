all: frontend backend

.PHONY: api-contract
api-contract:
	cd fermentbox-frontend && yarn install --ignore-engines
	cd fermentbox-frontend && yarn api:check

.PHONY: test-frontend
test-frontend:
	cd fermentbox-frontend && yarn install --ignore-engines
	cd fermentbox-frontend && yarn test

.PHONY: frontend
frontend:
	cd fermentbox-frontend && yarn install
	cd fermentbox-frontend && yarn api:check
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
