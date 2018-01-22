CREATE TABLE IF NOT EXISTS info (
	varname		TEXT UNIQUE  NOT NULL,
	vardata		TEXT
);

CREATE TABLE IF NOT EXISTS user (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50) NOT NULL,
	
	UNIQUE (username)
);

CREATE TABLE IF NOT EXISTS settings (
    owner       INTEGER NOT NULL,
	key			TEXT NOT NULL,
	value		TEXT,

	UNIQUE (owner, key),
    FOREIGN KEY (owner) REFERENCES user(id)
);

CREATE TABLE IF NOT EXISTS log (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner       INTEGER NOT NULL,
    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50) NOT NULL,
	message		TEXT,
    started_on	DATETIME NOT NULL,
    ended_on	DATETIME NOT NULL,
	type		INTEGER,
	
    FOREIGN KEY (owner) REFERENCES user(id)
);

CREATE TABLE IF NOT EXISTS file (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner		INTEGER NOT NULL,
	
	file		TEXT,
	
    FOREIGN KEY (owner) REFERENCES log(id)
);

CREATE TABLE IF NOT EXISTS contact (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner       INTEGER NOT NULL,

    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50),
    sdial    	VARCHAR(5),

    firstName	TEXT,
    lastName	TEXT,
    country		TEXT,
    state		TEXT,

    homepage	TEXT,
    email		TEXT,
    mobile		TEXT,
    office		TEXT,

	UNIQUE (owner, username),
    FOREIGN KEY (owner) REFERENCES user(id)
);

INSERT OR REPLACE INTO info VALUES('version', '0.2.0');
INSERT OR REPLACE INTO info VALUES('support-username',	'Tsupport');
INSERT OR REPLACE INTO info VALUES('support-vpnumber',	'252-800-2030');
