FROM ubuntu:18.04

RUN apt update
RUN apt install -y python3
RUN rm -rf /var/lib/apt/lists/*

RUN mkdir /sbu

COPY ./so /sbu/sbu_app/so
COPY ./sbu /sbu/sbu_app

COPY ./SBUWebApp /sbu/SBUWebApp
COPY ./start.sh /sbu/start.sh

COPY --from=web_sbu:1.0.0 /web_sbu/sbu_venv /venv

ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/sbu/sbu_app/so
ENV PATH $PATH:/sbu/sbu_app:/venv/bin

WORKDIR /sbu

CMD /sbu/start.sh
