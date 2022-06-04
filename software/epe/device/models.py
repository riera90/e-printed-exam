from django.db import models

vendors = ['483fda']

class Device(models.Model):
    def __str__(self):
        return self.uuid

    uuid = models.CharField(max_length=32, primary_key=True)
    vendor = models.CharField(max_length=32)
    name = models.CharField(max_length=1024, null=True, blank=True)
    token = models.CharField(max_length=1024, null=True, blank=True)
    classroom = models.ForeignKey('classroom.Classroom', related_name='devices', on_delete=models.DO_NOTHING, null=True, blank=True)
    last_seen = models.DateTimeField(auto_now=True)
    ttl = models.IntegerField()
