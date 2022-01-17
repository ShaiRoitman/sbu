FROM ubuntu:18.04

RUN mkdir /sbu 
RUN mkdir /work

COPY sbu_app/* /sbu/

ENV LD_LIBRARY_PATH /sbu
ENV PATH $PATH:/sbu

WORKDIR /work

CMD /bin/bash
