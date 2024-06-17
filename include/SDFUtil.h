#ifndef SDFUTIL_H
#define SDFUTIL_H

#include "raylib.h"
#include "raymath.h"
static int RayPlaneIntersection(const Vector3 RayOrigin, const Vector3 RayDirection, const Vector3 PlanePoint,
                                     const Vector3 PlaneNormal, Vector3* IntersectionPoint)
{
    float dotProduct =
        (PlaneNormal.x * RayDirection.x) + (PlaneNormal.y * RayDirection.y) + (PlaneNormal.z * RayDirection.z);

    // Check if the ray is parallel to the plane
    if (dotProduct == 0.0f)
    {
        return 0;
    }

    float t = ((PlanePoint.x - RayOrigin.x) * PlaneNormal.x + (PlanePoint.y - RayOrigin.y) * PlaneNormal.y +
               (PlanePoint.z - RayOrigin.z) * PlaneNormal.z) /
              dotProduct;

    IntersectionPoint->x = RayOrigin.x + t * RayDirection.x;
    IntersectionPoint->y = RayOrigin.y + t * RayDirection.y;
    IntersectionPoint->z = RayOrigin.z + t * RayDirection.z;

    return 1;
}


static Vector3 WorldToCamera(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, cameraMatrix);
}

static Vector3 CameraToWorld(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, MatrixInvert(cameraMatrix));
}

static Vector3 VectorProjection(const Vector3 vectorToProject, const Vector3 targetVector)
{
    float dotProduct = (vectorToProject.x * targetVector.x) + (vectorToProject.y * targetVector.y) +
                       (vectorToProject.z * targetVector.z);

    float targetVectorLengthSquared =
        (targetVector.x * targetVector.x) + (targetVector.y * targetVector.y) + (targetVector.z * targetVector.z);

    float scale = dotProduct / targetVectorLengthSquared;

    Vector3 projection;
    projection.x = targetVector.x * scale;
    projection.y = targetVector.y * scale;
    projection.z = targetVector.z * scale;

    return projection;
}

// Find the point on line p1 to p2 nearest to line p2 to p4
static Vector3 NearestPointOnLine(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
{
    float mua;

    Vector3 p13, p43, p21;
    float d1343, d4321, d1321, d4343, d2121;
    float numer, denom;

    const float EPS = 0.001;

    p13.x = p1.x - p3.x;
    p13.y = p1.y - p3.y;
    p13.z = p1.z - p3.z;
    p43.x = p4.x - p3.x;
    p43.y = p4.y - p3.y;
    p43.z = p4.z - p3.z;
    if (fabs(p43.x) < EPS && fabs(p43.y) < EPS && fabs(p43.z) < EPS)
        return (Vector3){};
    p21.x = p2.x - p1.x;
    p21.y = p2.y - p1.y;
    p21.z = p2.z - p1.z;
    if (fabs(p21.x) < EPS && fabs(p21.y) < EPS && fabs(p21.z) < EPS)
        return (Vector3){};

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (fabs(denom) < EPS)
        return (Vector3){};
    numer = d1343 * d4321 - d1321 * d4343;

    mua = numer / denom;

    return (Vector3){p1.x + mua * p21.x, p1.y + mua * p21.y, p1.z + mua * p21.z};
}

static BoundingBox boundingBoxSized(Vector3 center, float size)
{
    return (BoundingBox){
        Vector3SubtractValue(center, size / 2),
        Vector3AddValue(center, size / 2),
    };
}

static BoundingBox shapeBoundingBox(SDFObject s)
{
    // const float radius = sqrtf(powf(s.size.x, 2) + powf(s.size.y, 2) + powf(s.size.z, 2));
    return (BoundingBox){
        Vector3Subtract(s.pos, s.size),
        Vector3Add(s.pos, s.size),
    };
}

static Vector3 VertexInterp(Vector4 p1, Vector4 p2, float threshold)
{
    if (fabsf(threshold - p1.w) < 0.00001)
        return *(Vector3*)&p1;
    if (fabsf(threshold - p2.w) < 0.00001)
        return *(Vector3*)&p2;
    if (fabsf(p1.w - p2.w) < 0.00001)
        return *(Vector3*)&p1;

    float mu = (threshold - p1.w) / (p2.w - p1.w);
    Vector3 r = {
        p1.x + mu * (p2.x - p1.x),
        p1.y + mu * (p2.y - p1.y),
        p1.z + mu * (p2.z - p1.z),
    };

    return r;
}
#endif //SDFUTIL_H
