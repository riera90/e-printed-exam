from django.urls import path
from .views import *

urlpatterns = [
    path('', list),
    path('discover/', web_api_search_device),
    path('ack/', device_api_acknowledgment_device),
    path('create/', create),
    path('<str:id>/', detail),
    path('<str:id>/update/', update),
    path('<str:id>/delete/', delete),
]
