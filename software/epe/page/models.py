from django.db import models

Types = [
    ("T", "Texto"),
    ("I", "Imagen")
]

class Page(models.Model):
    document = models.ForeignKey('document.Document', related_name='pages', on_delete=models.CASCADE)
    order = models.IntegerField()
    title = models.CharField(max_length=256)
    binary_image = models.BinaryField(null=True, blank=True)
    h = models.IntegerField(default=0)
    w = models.IntegerField(default=0)
    body = models.TextField(default="")
    type = models.CharField(max_length=8, choices=Types, default="T")
