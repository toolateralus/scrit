#pragma once

#include "raylib.h"
#include "raymath.h"

#include <scrit/context.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>

static Object CreateVector3(Vector3 &v) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["x"] = Ctx::CreateFloat(v.x);
  obj->scope->variables["y"] = Ctx::CreateFloat(v.y);
  obj->scope->variables["z"] = Ctx::CreateFloat(v.z);
  return obj;
}
static Object CreateQuaternion(Quaternion &quat) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["x"] = Ctx::CreateFloat(quat.x);
  obj->scope->variables["y"] = Ctx::CreateFloat(quat.y);
  obj->scope->variables["z"] = Ctx::CreateFloat(quat.z);
  obj->scope->variables["w"] = Ctx::CreateFloat(quat.w);
  return obj;
}
static Object CreateTransform(Transform &transform) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["position"] = CreateVector3(transform.translation);
  obj->scope->variables["rotation"] = CreateQuaternion(transform.rotation);
  obj->scope->variables["scale"] = CreateVector3(transform.scale);
  return obj;
}
static Object CreateMatrix(Matrix &transform) {
  Object obj = Ctx::CreateObject();
  
  // Extract translation
  Vector3 translation = {transform.m12, transform.m13, transform.m14};
  obj->scope->variables["position"] = CreateVector3(translation);

  // Extract scale
  Vector3 scale;
  scale.x = Vector3Length((Vector3){transform.m0, transform.m1, transform.m2});
  scale.y = Vector3Length((Vector3){transform.m4, transform.m5, transform.m6});
  scale.z = Vector3Length((Vector3){transform.m8, transform.m9, transform.m10});
  obj->scope->variables["scale"] = CreateVector3(scale);

  // Normalize the matrix to extract rotation
  Matrix rotationMatrix = transform;
  rotationMatrix.m0 /= scale.x;
  rotationMatrix.m1 /= scale.x;
  rotationMatrix.m2 /= scale.x;
  rotationMatrix.m4 /= scale.y;
  rotationMatrix.m5 /= scale.y;
  rotationMatrix.m6 /= scale.y;
  rotationMatrix.m8 /= scale.z;
  rotationMatrix.m9 /= scale.z;
  rotationMatrix.m10 /= scale.z;

  // Convert the rotation matrix to a quaternion
  Quaternion rotation = QuaternionFromMatrix(rotationMatrix);
  obj->scope->variables["rotation"] = CreateQuaternion(rotation);

  return obj;
}

static bool TryGetVector3(Object value, Vector3& vector) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;
  auto xval = scope->variables["x"];
  auto yval = scope->variables["y"];
  auto zval = scope->variables["z"];
  if (Ctx::IsUndefined(xval, yval, zval) ||
      Ctx::IsNull(xval, yval, zval)) {
    return false;
  }

  float x, y, z;
  if (!Ctx::TryGetFloat(xval, x) || !Ctx::TryGetFloat(yval, y) ||
      !Ctx::TryGetFloat(zval, z)) {
    return false;
  }

  vector = Vector3{x, y, z};
  return true;
}
static bool TryGetQuaternion(Object value, Quaternion& quat) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;
  auto xval = scope->variables["x"];
  auto yval = scope->variables["y"];
  auto zval = scope->variables["z"];
  auto wval = scope->variables["w"];
  if (Ctx::IsUndefined(xval, yval, zval, wval) ||
      Ctx::IsNull(xval, yval, zval, wval)) {
    return false;
  }

  float x, y, z, w;
  if (!Ctx::TryGetFloat(xval, x) || !Ctx::TryGetFloat(yval, y) ||
      !Ctx::TryGetFloat(zval, z) || !Ctx::TryGetFloat(wval, w)) {
    return false;
  }

  quat = Quaternion{x, y, z, w};
  return true;
}
static bool TryGetMatrix(Object value, Matrix& matrix) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }
  
  auto scope = value->scope;
  auto positionVal = scope->variables["position"];
  auto scaleVal = scope->variables["scale"];
  auto rotationVal = scope->variables["rotation"];
  
  Object posObj;
  Object scaleObj;
  Object rotationObj;
  if (!Ctx::TryGetObject(positionVal, posObj) || !Ctx::TryGetObject(scaleVal, scaleObj) ||
      !Ctx::TryGetObject(rotationVal, rotationObj)) {
    return false;
  }
  
  Vector3 position;
  Vector3 scale;
  Quaternion rotation;
  if (!TryGetVector3(posObj, position) ||
      !TryGetVector3(scaleObj, scale) ||
      !TryGetQuaternion(rotationObj, rotation)) {
    return false;
  }
  
  // Create the transformation matrix
  matrix = MatrixIdentity();
  matrix = MatrixMultiply(matrix, MatrixTranslate(position.x, position.y, position.z));
  matrix = MatrixMultiply(matrix, MatrixScale(scale.x, scale.y, scale.z));
  matrix = MatrixMultiply(matrix, QuaternionToMatrix(rotation));
  return true;
}
static bool TryGetColor(Object value, Color &color) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;
  
  auto rval = value->GetMember("r");
  auto gval = value->GetMember("g");
  auto bval = value->GetMember("b");
  auto aval = value->GetMember("a");

  if (Ctx::IsUndefined(rval, gval, bval, aval) ||
      Ctx::IsNull(rval, gval, bval, aval)) {
    return false;
  }

  int r, g, b, a;
  if (!Ctx::TryGetInt(rval, r) || !Ctx::TryGetInt(gval, g) ||
      !Ctx::TryGetInt(bval, b) || !Ctx::TryGetInt(aval, a)) {
    return false;
  }

  color = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b,
                (unsigned char)a};

  return true;
}