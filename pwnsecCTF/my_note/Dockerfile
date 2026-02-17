FROM ubuntu:22.04@sha256:b6b83d3c331794420340093eb706a6f152d9c1fa51b262d9bf34594887c2c7ac

RUN apt update && apt-get install -y socat
RUN adduser --disabled-login --gecos '' -- "pwn"

WORKDIR /home/pwn
USER pwn

COPY ./prob ./prob
COPY ./flag ./flag

USER root
RUN chmod 444 ./flag
RUN chmod 755 ./prob

USER pwn
EXPOSE 13131
CMD ["socat", "TCP-LISTEN:13131,reuseaddr,fork", "EXEC:./prob,stderr"]