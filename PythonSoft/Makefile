.PHONY: docs clean upload build

default: build

docs: ## Update python docs
	@cd docs/sphinx; sphinx-apidoc -f -o ./source ../../moller
	@cd docs/sphinx; make html

clean: ## Clean python cache files
	@rm -rf output/* moller/__pycache__
	@cd docs/sphinx; make clean

build:
	@python -m build -o output

test: ## Build and upload python scripts to testpypi
	@make clean build
	@rm -rf output/*.egg-info/
	@twine upload -r testpypi output/* || true

upload: ## Build and upload python scripts
	@make clean build
	@rm -rf output/*.egg-info/
	@twine upload -r pypi output/* || true
