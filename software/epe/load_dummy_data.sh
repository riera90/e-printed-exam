#!/bin/sh
echo "Removing pycache."
find . | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
echo "Removing database."
rm ./db.sqlite3
# Make migrations and migrate the database.
echo "Making migrations and migrating the database. "
python manage.py makemigrations --noinput
python manage.py migrate auth --noinput
python manage.py migrate --noinput --run-syncdb
#python manage.py createsuperuser --username admin --email admin@contapy.com --skip-checks
python manage.py shell < ./dummy.py
