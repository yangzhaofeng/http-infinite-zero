FROM python:3-alpine

WORKDIR /usr/src/app

COPY host.py ./

CMD [ "python", "./host.py" ]
