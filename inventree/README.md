# Inventree

- Basic PLM and inventory tracking software built on django
- Does BOMs and parts consumption quite well

- [Repo](http://github.com/inventree/InvenTree)

## Running Server

Composed of 5x processes:

- Redis Cache
- Worker
- Server
- Proxy
- Postgres

1. Clone the repo

```sh
git clone git@github.com:inventree/InvenTree.git
```

2. Start all the servers

```sh
docker compose up
```

3. Configure Database

## Database Maintenance

### Access Database

The settings for this is set in docker-compose.yml

```yml
environment:
  - PGDATA=/var/lib/postgresql/data/pgdb
  - POSTGRES_USER=pguser
  - POSTGRES_PASSWORD=pgpassword
  - POSTGRES_DB=inventree
```

```sh
docker exec -it -u postgres inventree-db psql -U pguser inventree
```

4. Backup Database

- <https://docs.inventree.org/en/stable/start/backup/>

Backups can also be forced:

```sh
docker compose --project-directory <DOCKER_CONFIG_DIRECTORY> exec -it inventree-server invoke backup
```

This will produce two files with timestamps under `/home/inventree/data/backup`:

- `default-33fab5aa6d30-2025-11-03-185046.psql.bin.gz`
- `33fab5aa6d30-2025-11-03-185050.tar.gz`

Note that running with the `-p` argument exports both files under the same name so the
database backup is overwritten by the file export. This probably calls for an issue report.

Calling backup with `exec` instead of `compose` will only backup the server data and not the database.

## Configuration

### Logo Images

- Replace /home/inventree/data/static/img/inventree.png and /home/inventree/data/static/web/inventree.svg with custom logos.
  - /home/inventree/data/static/img/inventree.png is used as the report logo and expects a 2083 pixel wide logo.
