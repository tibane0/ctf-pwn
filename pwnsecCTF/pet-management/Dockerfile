FROM ubuntu:24.04@sha256:4f1db91d9560cf107b5832c0761364ec64f46777aa4ec637cca3008f287c975e

RUN apt-get update && apt-get install -y --no-install-recommends \
    socat coreutils netcat-openbsd libseccomp2 && \
    rm -rf /var/lib/apt/lists/*

RUN install -d -m 755 /srv/app

COPY chall /srv/app/chall
RUN chmod 0555 /srv/app/chall

COPY run.sh /srv/app/run.sh
RUN chmod 0755 /srv/app/run.sh

RUN useradd -ms /bin/bash pwn

WORKDIR /srv/app
EXPOSE 5050

ENTRYPOINT ["/srv/app/run.sh"]

