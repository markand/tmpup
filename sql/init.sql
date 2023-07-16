create table if not exists `paste`(
	`id`            TEXT PRIMARY KEY,
	`title`         TEXT not NULL,
	`author`        TEXT not NULL,
	`filename`      TEXT not NULL,
	`language`      TEXT not NULL,
	`code`          TEXT not NULL,
	`start`         INTEGER not NULL,
	`end`           INTEGER not NULL
) STRICT;

create table if not exists `image`(
	`id`            TEXT PRIMARY KEY,
	`title`         TEXT not NULL,
	`author`        TEXT not NULL,
	`filename`      TEXT not NULL,
	`data`          TEXT not NULL,
	`start`         INTEGER not NULL,
	`end`           INTEGER not NULL
) STRICT;

create table if not exists `setting`(
	`key`           TEXT PRIMARY KEY,
	`value`         TEXT not NULL
) STRICT;
