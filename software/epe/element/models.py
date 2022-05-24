from django.db import models

class Element(models.Model):

    page = models.ForeignKey('page.Page', related_name='elements', on_delete=models.CASCADE)
    raw_data_red = models.BinaryField()
    raw_data_black = models.BinaryField()
    text = models.TextField()
    font = models.CharField(max_length=256)
    x = models.IntegerField()
    y = models.IntegerField()
    w = models.IntegerField()
    h = models.IntegerField()
