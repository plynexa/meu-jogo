"""Cria um banco NOVO de teste local; recusa sobrescrever banco existente."""
from pathlib import Path
import sqlite3, sys, hashlib
schema = Path(__file__).parents[1] / 'servidor/schema-reference.sql'
target = Path(sys.argv[1]) if len(sys.argv)>1 else Path('meujogo.s3db')
if target.exists():
    raise SystemExit('Banco ja existe; preservado: '+str(target))
db=sqlite3.connect(target)
try:
    db.executescript(schema.read_text(encoding='utf8'))
    # Transicoes 31 -> 43 conforme databasemanager.cpp, ramo SQLite.
    db.executescript('''
CREATE TABLE market_offers (id INTEGER PRIMARY KEY, player_id INTEGER NOT NULL, sale BOOLEAN NOT NULL DEFAULT 0, itemtype INTEGER NOT NULL, amount INTEGER NOT NULL, created INTEGER NOT NULL, anonymous BOOLEAN NOT NULL DEFAULT 0, price INTEGER NOT NULL DEFAULT 0);
CREATE INDEX market_offers_idx ON market_offers(created);
CREATE INDEX market_offers_idx2 ON market_offers(sale,itemtype);
CREATE TABLE market_history (id INTEGER PRIMARY KEY, player_id INTEGER NOT NULL, sale BOOLEAN NOT NULL DEFAULT 0, itemtype INTEGER NOT NULL, amount INTEGER NOT NULL, price INTEGER NOT NULL DEFAULT 0, expires_at INTEGER NOT NULL, inserted INTEGER NOT NULL, state INTEGER NOT NULL);
CREATE INDEX market_history_idx ON market_history(player_id,sale);
CREATE TABLE guild_wars (id INTEGER PRIMARY KEY, guild_id INTEGER NOT NULL, enemy_id INTEGER NOT NULL, begin INTEGER NOT NULL DEFAULT 0, end INTEGER NOT NULL DEFAULT 0, frags INTEGER NOT NULL DEFAULT 0, payment INTEGER NOT NULL DEFAULT 0, guild_kills INTEGER NOT NULL DEFAULT 0, enemy_kills INTEGER NOT NULL DEFAULT 0, status INTEGER NOT NULL DEFAULT 0);
CREATE TABLE guild_kills (id INTEGER PRIMARY KEY, guild_id INTEGER NOT NULL, war_id INTEGER NOT NULL, death_id INTEGER NOT NULL);
ALTER TABLE guilds ADD balance BIGINT NOT NULL DEFAULT 0;
ALTER TABLE killers ADD war BIGINT NOT NULL DEFAULT 0;
CREATE TABLE login_history (account_id INTEGER NOT NULL DEFAULT 0, player_id INTEGER NOT NULL DEFAULT 0, type INTEGER NOT NULL DEFAULT 0, login INTEGER NOT NULL DEFAULT 0, ip INTEGER NOT NULL DEFAULT 0, date INTEGER NOT NULL DEFAULT 0);
ALTER TABLE players ADD ip VARCHAR(17) NOT NULL DEFAULT '0';
ALTER TABLE players ADD offlinetraining_time INTEGER NOT NULL DEFAULT 43200;
ALTER TABLE players ADD offlinetraining_skill INTEGER NOT NULL DEFAULT -1;
ALTER TABLE players ADD broadcasting BOOLEAN NOT NULL DEFAULT 0;
ALTER TABLE players ADD viewers INTEGER NOT NULL DEFAULT 0;
''')
    for table in ['player_items','player_depotitems','tile_items','tile_store','house_data']:
        db.execute(f'ALTER TABLE {table} ADD serial VARCHAR(255) NOT NULL DEFAULT \'\'')
    db.execute("UPDATE server_config SET value='43' WHERE config='db_version'")
    db.execute("UPDATE accounts SET name='teste',password=?,salt='',premdays=0 WHERE id=1",(hashlib.sha1(b'teste123').hexdigest(),))
    db.execute("UPDATE players SET name='Explorador',group_id=1,vocation=1,level=8,health=185,healthmax=185,mana=100,manamax=100,town_id=1,posx=1043,posy=1048,posz=5,looktype=110,save=1 WHERE id=1")
    for skill in range(7):
        db.execute('INSERT OR IGNORE INTO player_skills(player_id,skillid,value,count) VALUES(1,?,10,0)',(skill,))
    db.commit()
    assert db.execute('PRAGMA integrity_check').fetchone()[0]=='ok'
    assert db.execute("SELECT count(*) FROM players WHERE name='Explorador'").fetchone()[0]==1
    print('Banco novo criado; integridade OK:',target)
finally:
    db.close()
