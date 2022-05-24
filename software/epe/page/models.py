from django.db import models

class Page(models.Model):
    document = models.ForeignKey('document.Document', related_name='pages', on_delete=models.CASCADE)
    order = models.IntegerField()
