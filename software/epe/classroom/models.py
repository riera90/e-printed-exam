from django.db import models

class Classroom(models.Model):
    def __str__(self):
        return self.name

    code = models.CharField(max_length=256, primary_key=True)
    name = models.CharField(max_length=1024)
