from rest_framework import permissions


class discover_device_permissions(permissions.BasePermission):
    def has_object_permission(self, request, view, obj):
        if request.method == 'GET':
            return not request.user.is_anonymous
        elif request.method == 'POST':
            return request.user.is_staff
        return False


class AreaDetailPermissions(permissions.BasePermission):
    def has_object_permission(self, request, view, obj):
        if request.method == 'GET':
            return not request.user.is_anonymous
        elif request.method in ['PUT', 'PATCH']:
            return request.user.is_staff
        elif request.method == 'DELETE':
            return request.user.is_staff
        return False