#!/bin/bash

VERSION=$(TZ=UTC date +%Y%m%d-%H%M%S)
DATE=$(TZ=UTC date -R)

cat debian/changelog.template | sed "s/VERSION/${VERSION}/g" | sed "s/DATE/${DATE}/g" > debian/changelog
