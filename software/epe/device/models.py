from django.db import models

class Device(models.Model):
    def __str__(self):
        return self.uuid

    uuid = models.CharField(max_length=1024, primary_key=True)
    name = models.CharField(max_length=1024)
    token = models.CharField(max_length=1024)
    classroom = models.ForeignKey('classroom.Classroom', related_name='devices', on_delete=models.DO_NOTHING)
