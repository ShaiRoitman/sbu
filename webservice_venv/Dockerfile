FROM ubuntu:18.04

RUN apt update
RUN apt install -y python3 python3-pip
RUN apt install -y python3-venv

RUN mkdir /venv

WORKDIR /venv

RUN python3 -m venv /venv /venv/webservice_venv

COPY ./requirements.txt /venv/webservice_venv/requirements.txt
RUN  /venv/webservice_venv/bin/python3 -m pip install -r /venv/webservice_venv/requirements.txt

CMD /bin/bash

