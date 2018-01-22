CREATE TABLE IF NOT EXISTS info (
        varname         TEXT UNIQUE  NOT NULL,
        vardata         TEXT
);

CREATE TABLE IF NOT EXISTS user (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50) NOT NULL,

        UNIQUE (username)
);

CREATE TABLE IF NOT EXISTS settings (
    owner       INTEGER NOT NULL,
        key                     TEXT NOT NULL,
        value           TEXT,

        UNIQUE (owner, key),
    FOREIGN KEY (owner) REFERENCES user(id)
);

CREATE TABLE IF NOT EXISTS log (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner       INTEGER NOT NULL,
    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50) NOT NULL,
        message         TEXT,
    started_on  DATETIME NOT NULL,
    ended_on    DATETIME NOT NULL,
        type            INTEGER,

    FOREIGN KEY (owner) REFERENCES user(id)
);

CREATE TABLE IF NOT EXISTS file (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner               INTEGER NOT NULL,

        file            TEXT,

    FOREIGN KEY (owner) REFERENCES log(id)
);

CREATE TABLE IF NOT EXISTS contact (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner       INTEGER NOT NULL,

    username    VARCHAR(50) NOT NULL,
    vpnumber    VARCHAR(50),
    sdial       VARCHAR(5),

    firstName   TEXT,
    lastName    TEXT,
    country             TEXT,
    state               TEXT,

    homepage    TEXT,
    email               TEXT,
    mobile              TEXT,
    office              TEXT,

        UNIQUE (owner, username),
    FOREIGN KEY (owner) REFERENCES user(id)
);

CREATE TABLE IF NOT EXISTS profiles (
	id          INTEGER PRIMARY KEY AUTOINCREMENT,

	login		VARCHAR(50) NOT NULL,
	password	VARCHAR(50) NOT NULL,
	number		VARCHAR(50),
	login_at	DOUBLE,
	first_name  TEXT,
	last_name   TEXT,
	email		TEXT,
	country		TEXT,
	state		TEXT,
	birthday	TEXT,
	gender		TEXT,
	phoneHome	TEXT,
	phoneOffice	TEXT,
	phoneMobile	TEXT,
	photoUpdate	TEXT,
	flags		INTEGER,
	status		INTEGER
);



CREATE TABLE IF NOT EXISTS calls (
	id          INTEGER PRIMARY KEY AUTOINCREMENT,
	seen		INTEGER NOT NULL,
	status		INTEGER NOT NULL,
	type		INTEGER NIT NULL,
	remote		TEXT,
	start		DOUBLE,
	end			DOUBLE
);


CREATE TABLE IF NOT EXISTS countries (
	id          INTEGER PRIMARY KEY AUTOINCREMENT,
	code		VARCHAR(10) NOT NULL,
	name		TEXT,
	iso2		VARCHAR(10) NOT NULL,
	idx			VARCHAR(10) NOT NULL
);



DELETE FROM countries;

INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('93', 'Afghanistan', 'AF', '197');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('358', 'Aland Islands', 'AX', '255');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('355', 'Albania', 'AL', '74');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('213', 'Algeria', 'DZ', '5');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('684', 'American Samoa', 'AS', '156');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('376', 'Andorra', 'AD', '221');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('244', 'Angola', 'AO', '32');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Anguilla', 'AI', '172');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('672', 'Antarctica', 'AQ', '144');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Antigua and Barbuda', 'AG', '222');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('54', 'Argentina', 'AR', '120');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('374', 'Armenia', 'AM', '83');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('297', 'Aruba', 'AW', '61');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('247', 'Ascension', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('61', 'Australia', 'AU', '136');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('672', 'Australian External Territories', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('43', 'Austria', 'AT', '99');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('994', 'Azerbaijan', 'AZ', '219');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Bahamas', 'BS', '175');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('973', 'Bahrain', 'BH', '212');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('880', 'Bangladesh', 'BD', '191');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Barbados', 'BB', '176');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('375', 'Belarus', 'BY', '84');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('32', 'Belgium', 'BE', '66');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('501', 'Belize', 'BZ', '107');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('229', 'Benin', 'BJ', '17');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Bermuda', 'BM', '177');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('975', 'Bhutan', 'BT', '214');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('591', 'Bolivia', 'BO', '126');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('387', 'Bosnia and Herzegovina', 'BA', '90');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('267', 'Botswana', 'BW', '54');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'Bouvet Islands', 'BV', '257');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('55', 'Brazil', 'BR', '121');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'British Indian Ocean Territory', 'IO', '258');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('673', 'Brunei Darussalm', 'BN', '145');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('359', 'Bulgaria', 'BG', '78');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('226', 'Burkina Faso (former Upper Volta)', 'BF', '14');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('257', 'Burundi', 'BI', '45');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('855', 'Cambodia (Kingdom of)', 'KH', '223');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('237', 'Cameroon', 'CM', '25');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Canada', 'CA', '2');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('238', 'Cape Verde', 'CV', '26');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Cayman Islands', 'KY', '224');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('236', 'Central African Republic', 'CF', '24');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('235', 'Chad', 'TD', '23');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('56', 'Chile', 'CL', '122');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('86', 'China', 'CN', '187');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('61', 'Christmas Island', 'CX', '225');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('61', 'Cocos (Keeling) Islands', 'CC', '226');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('57', 'Colombia', 'CO', '123');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('269', 'Comoros', 'KM', '270');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('242', 'Congo', 'CG', '30');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('243', 'Congo The Democratic Republic of the', 'CD', '31');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('682', 'Cook Islands', 'CK', '154');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('506', 'Costa Rica', 'CR', '112');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('225', 'Cote dIvoire', 'CI', '260');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('385', 'Croatia', 'HR', '89');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('53', 'Cuba', 'CU', '118');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('357', 'Cyprus', 'CY', '76');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('420', 'Czech Republic', 'CZ', '227');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('45', 'Denmark', 'DK', '101');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('246', 'Diego Garcia', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('253', 'Djibouti', 'DJ', '41');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Dominica', 'DM', '228');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Dominican Republic', 'DO', '229');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('593', 'Ecuador', 'EC', '128');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('20', 'Egypt', 'EG', '3');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('503', 'El Salvador', 'SV', '109');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('240', 'Equatorial Guinea', 'GQ', '28');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('291', 'Eritrea', 'ER', '230');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('372', 'Estonia', 'EE', '231');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('251', 'Ethiopia', 'ET', '39');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('500', 'Falkland Islands(Malvinas)', 'FK', '106');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('298', 'Faroe Islands', 'FO', '62');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('679', 'Fiji', 'FJ', '151');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('358', 'Finland', 'FI', '77');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('33', 'France', 'FR', '67');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('594', 'French Guiana', 'GF', '129');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('689', 'French Polynesia', 'PF', '232');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'French Southern Territories', 'TF', '261');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('241', 'Gabon', 'GA', '29');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('220', 'Gambia', 'GM', '8');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('995', 'Georgia', 'GE', '220');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('49', 'Germany', 'DE', '105');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('233', 'Ghana', 'GH', '21');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('350', 'Gibraltar', 'GI', '69');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('30', 'Greece', 'GR', '64');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('299', 'Greenland', 'GL', '63');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Grenada', 'GD', '233');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('388', 'Group of countries shared code', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('590', 'Guadeloupe', 'GP', '234');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Guam', 'GU', '143');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('502', 'Guatemala', 'GT', '108');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('224', 'Guinea', 'GN', '12');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('245', 'Guinea-Bissau', 'GW', '33');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('592', 'Guyana', 'GY', '127');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('509', 'Haiti', 'HT', '115');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'Heard Island And Mcdonald Islands', 'HM', '262');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('39', 'Holy See (Vatican City State)', 'VA', '94');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('379', 'Holy See (Vatican City State)', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('504', 'Honduras', 'HN', '110');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('852', 'Hong Kong', 'HK', '183');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('36', 'Hungary', 'HU', '79');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('354', 'Iceland', 'IS', '73');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('91', 'India', 'IN', '194');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('62', 'Indonesia', 'ID', '137');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('871', 'Inmarsat (Atlantic Ocean-East)', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('874', 'Inmarsat (Atlantic Ocean-West)', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('873', 'Inmarsat (Indian Ocean)', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('872', 'Inmarsat (Pacific Ocean)', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('870', 'Inmarsat SNAC', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('800', 'International Freephone Service', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('881', 'International Mobile shared code', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('882', 'International Networks shared code', '', '');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('98', 'Iran Islamic Republic Of', 'IR', '217');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('964', 'Iraq', 'IQ', '204');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('353', 'Ireland', 'IE', '72');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('972', 'Israel', 'IL', '211');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('39', 'Italy', 'IT', '93');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Jamaica', 'JM', '235');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('81', 'Japan', 'JP', '179');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('962', 'Jordan', 'JO', '202');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('7', 'Kazakhstan', 'KZ', '166');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('254', 'Kenya', 'KE', '42');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('686', 'Kiribati', 'KI', '158');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('850', "Korea Democratic People's Republic Of", 'KP', '182');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('82', 'Korea Republic of', 'KR', '180');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('965', 'Kuwait', 'KW', '205');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('7', 'Kyrgystan', 'KG', '167');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('856', 'Lao Peoples Democratic Republic', 'LA', '236');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('371', 'Latvia', 'LV', '81');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('961', 'Lebanon', 'LB', '201');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('266', 'Lesotho', 'LS', '53');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('231', 'Liberia', 'LR', '19');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('218', 'Libyan Arab Jamahiriya', 'LY', '237');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('423', 'Liechtenstein', 'LI', '96');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('370', 'Lithuania', 'LT', '80');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('352', 'Luxembourg', 'LU', '71');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('853', 'Macao', 'MO', '184');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('389', 'Macedonia The Former Yugoslav Republic Of', 'MK', '238');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('261', 'Madagascar', 'MG', '48');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('265', 'Malawi', 'MW', '52');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('60', 'Malaysia', 'MY', '135');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('960', 'Maldives', 'MV', '200');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('223', 'Mali', 'ML', '11');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('356', 'Malta', 'MT', '75');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('692', 'Marshall Islands', 'MH', '164');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('596', 'Martinique', 'MQ', '131');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('222', 'Mauritania', 'MR', '10');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('230', 'Mauritius', 'MU', '18');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('269', 'Mayotte', 'YT', '56');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('52', 'Mexico', 'MX', '117');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('691', 'Micronesia Federated States Of', 'FM', '163');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('373', 'Moldova Republic Of', 'MD', '82');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('377', 'Monaco', 'MC', '85');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('976', 'Mongolia', 'MN', '215');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Montserrat', 'MS', '239');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('212', 'Morocco', 'MA', '4');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('258', 'Mozambique', 'MZ', '46');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('95', 'Myanmar', 'MM', '199');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('264', 'Namibia', 'NA', '51');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('674', 'Nauru', 'NR', '146');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('977', 'Nepal', 'NP', '216');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('31', 'Netherlands', 'NL', '65');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('599', 'Netherlands Antilles', 'AN', '134');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('687', 'New Caledonia', 'NC', '159');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('64', 'New Zealand', 'NZ', '139');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('505', 'Nicaragua', 'NI', '111');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('227', 'Niger', 'NE', '15');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('234', 'Nigeria', 'NG', '22');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('683', 'Niue', 'NU', '155');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('672', 'Norfolk Island', 'NF', '240');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'North Mariana Islands', 'MP', '142');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('47', 'Norway', 'NO', '103');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('968', 'Oman', 'OM', '208');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('92', 'Pakistan', 'PK', '196');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('680', 'Palau', 'PW', '152');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('970', 'Palestinian Territory occupied', 'PS', '273');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('507', 'Panama', 'PA', '113');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('675', 'Papua New Guinea', 'PG', '147');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('595', 'Paraguay', 'PY', '130');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('51', 'Peru', 'PE', '116');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('63', 'Philippines', 'PH', '138');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'Pitcairn', 'PN', '271');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('48', 'Poland', 'PL', '104');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('351', 'Portugal', 'PT', '70');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Puerto Rico', 'PR', '241');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('974', 'Qatar', 'QA', '213');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('262', 'Reunion', 'RE', '49');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('40', 'Romania', 'RO', '95');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('7', 'Russia Federation', 'RU', '168');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('250', 'Rwanda', 'RW', '38');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('290', 'Saint Helena', 'SH', '58');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Saint Kitts And Nevis', 'KN', '242');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Saint Lucia', 'LC', '243');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('508', 'Saint Pierre And Miquelon', 'PM', '114');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Saint Vincent And The Grenadies', 'VC', '244');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('685', 'Samoa', 'WS', '245');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('378', 'San Marino', 'SM', '59');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('239', 'Sao Tome and Principe', 'ST', '27');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('966', 'Saudi Arabia', 'SA', '206');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('221', 'Senegal', 'SN', '9');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('381', 'Serbia And Montenegro', 'CS', '246');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('248', 'Seychelles', 'SC', '36');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('232', 'Sierra Leone', 'SL', '20');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('65', 'Singapore', 'SG', '140');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('421', 'Slovakia', 'SK', '250');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('386', 'Slovenia', 'SI', '272');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('677', 'Solomon Islands', 'SB', '149');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('252', 'Somalia', 'SO', '40');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('27', 'South Africa', 'ZA', '57');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('995', 'South Georgia And The South Sandwich Islands', 'GS', '274');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('34', 'Spain', 'ES', '68');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('94', 'Sri Lanka', 'LK', '198');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('249', 'Sudan', 'SD', '37');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('597', 'Suriname', 'SR', '132');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('47', 'Svalbard And Jan Mayen', 'SJ', '264');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('268', 'Swaziland', 'SZ', '55');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('46', 'Sweden', 'SE', '102');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('41', 'Switzerland', 'CH', '97');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('963', 'Syrian Arab Republic (Syria)', 'SY', '203');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('886', 'Taiwan -Province Of China', 'TW', '192');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('992', 'Tajikistan', 'TJ', '169');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('255', 'Tanzania United Republic Of', 'TZ', '43');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('66', 'Thailand', 'TH', '141');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('670', 'Timor-Leste', 'TL', '253');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('228', 'Togo', 'TG', '16');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('690', 'Tokelau', 'TK', '162');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('676', 'Tonga', 'TO', '148');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Trinidad and Tobago', 'TT', '60');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('216', 'Tunisia', 'TN', '6');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('90', 'Turkey', 'TR', '193');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('993', 'Turkmenistan', 'TM', '218');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Turks And Caicos Islands', 'TC', '254');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('688', 'Tuvalu (Ellice Islands)', 'TV', '160');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('256', 'Uganda', 'UG', '44');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('380', 'Ukraine', 'UA', '87');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('971', 'United Arab Emirates', 'AE', '210');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('44', 'United Kingdom', 'GB', '100');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'United States', 'US', '1');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('', 'United States Minor Outlying Islands', 'UM', '266');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('598', 'Uruguay', 'UY', '133');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('998', 'Uzbekistan', 'UZ', '170');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('678', 'Vanuatu', 'VU', '150');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('58', 'Venezuela', 'VE', '124');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('84', 'Viet Nam', 'VN', '181');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Virgin Islands British', 'VG', '178');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('1', 'Virgin Islands U.S', 'VI', '256');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('681', 'Wallis and Futuna', 'WF', '153');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('212', 'Western Sahara', 'EH', '267');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('967', 'Yemen', 'YE', '209');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('260', 'Zambia', 'ZM', '47');
INSERT OR REPLACE INTO countries (code, name, iso2, idx) VALUES('263', 'Zimbabwe', 'ZW', '50');


INSERT OR REPLACE INTO info VALUES('version', '0.2.0');
INSERT OR REPLACE INTO info VALUES('support-username',  'Tsupport');
INSERT OR REPLACE INTO info VALUES('support-vpnumber',  '252-800-2030');

/*INSERT OR REPLACE INTO contact (owner, username, firstName,lastName) VALUES (1, 'voip2cartest', 'Voip', 'Car');*/
/*INSERT OR REPLACE INTO contact (owner, username, firstName,lastName) VALUES (1, 'VIC_test', 'VIC', 'test');*/
/*INSERT OR REPLACE INTO contact (owner, username, firstName,lastName) VALUES (1, 'VIC_TEST2', 'VIC', 'Test2');*/

/*INSERT OR REPLACE INTO profiles (first_name, last_name) VALUES ('Voip', 'Car');*/

/*INSERT OR REPLACE INTO calls (seen,status,type,remote,start,end) VALUES (0,1,0,'VIC_TEST', 0, 0);*/




