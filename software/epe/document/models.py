from django.db import models
from django.contrib.auth.models import User


class Document(models.Model):
    def __str__(self):
        return 'documento '+self.name+' de '+self.subject_code+' para la case '+self.classroom.__str__()

    name = models.CharField(max_length=1024)
    description = models.TextField()
    subject_code = models.CharField(max_length=256)
    start = models.DateTimeField()
    end = models.DateTimeField()
    classroom = models.ForeignKey('classroom.Classroom', related_name='documents', on_delete=models.CASCADE)
    owner = models.ForeignKey(User, related_name='documents', on_delete=models.CASCADE)

