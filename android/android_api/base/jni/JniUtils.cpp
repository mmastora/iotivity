/*
* //******************************************************************
* //
* // Copyright 2015 Intel Corporation.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* //
* // Licensed under the Apache License, Version 2.0 (the "License");
* // you may not use this file except in compliance with the License.
* // You may obtain a copy of the License at
* //
* //      http://www.apache.org/licenses/LICENSE-2.0
* //
* // Unless required by applicable law or agreed to in writing, software
* // distributed under the License is distributed on an "AS IS" BASIS,
* // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* // See the License for the specific language governing permissions and
* // limitations under the License.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

#include "JniUtils.h"
#include "JniOcRepresentation.h"
using namespace std;

jobject JniUtils::convertStrVectorToJavaStrList(JNIEnv *env, std::vector<string> &vector)
{
    jobject jList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    if (env->ExceptionCheck()) return nullptr;
    for (size_t i = 0; i < vector.size(); ++i)
    {
        jstring str = env->NewStringUTF(vector[i].c_str());
        env->CallBooleanMethod(jList, g_mid_LinkedList_add_object, str);
        env->DeleteLocalRef(str);
        if (env->ExceptionCheck()) return nullptr;
    }
    return jList;
}

void JniUtils::convertJavaStrArrToStrVector(JNIEnv *env, jobjectArray jStrArr, std::vector<string> &vector)
{
    if (!jStrArr) return;

    jsize len = env->GetArrayLength(jStrArr);
    for (jsize i = 0; i < len; ++i)
    {
        jobject jObj = env->GetObjectArrayElement(jStrArr, i);
        if (!jObj) continue;

        jstring jStr = (jstring)jObj;
        vector.push_back(env->GetStringUTFChars(jStr, NULL));

        env->DeleteLocalRef(jObj);
    }
}

void JniUtils::convertJavaHeaderOptionsArrToVector(JNIEnv *env, jobjectArray jHeaderOptions,
    OC::HeaderOptions &headerOptions)
{
    if (!jHeaderOptions) return;

    jsize len = env->GetArrayLength(jHeaderOptions);

    for (jsize i = 0; i < len; ++i)
    {
        jobject header = env->GetObjectArrayElement(jHeaderOptions, i);
        jint jId = env->CallIntMethod(header, g_mid_OcHeaderOption_get_id);
        jobject jobj = (jstring)env->CallObjectMethod(header, g_mid_OcHeaderOption_get_data);
        if (!jobj) continue;
        jstring jData = (jstring)jobj;

        OC::HeaderOption::OCHeaderOption hopt(
            static_cast<int>(jId),
            env->GetStringUTFChars(jData, NULL));

        headerOptions.push_back(hopt);

        env->DeleteLocalRef(header);
        env->DeleteLocalRef(jobj);
        if (env->ExceptionCheck()) return;
    }
}

jobject JniUtils::convertHeaderOptionsVectorToJavaList(JNIEnv *env, const OC::HeaderOptions& headerOptions)
{
    jobject jHeaderOptionList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    for (size_t i = 0; i < headerOptions.size(); ++i)
    {
        jobject jHeaderOption = env->NewObject(
            g_cls_OcHeaderOption,
            g_mid_OcHeaderOption_ctor,
            static_cast<jint>(headerOptions[i].getOptionID()),
            env->NewStringUTF(headerOptions[i].getOptionData().c_str())
            );

        env->CallBooleanMethod(jHeaderOptionList, g_mid_LinkedList_add_object, jHeaderOption);
        env->DeleteLocalRef(jHeaderOption);
        if (env->ExceptionCheck()) return nullptr;
    }

    return jHeaderOptionList;
}


void JniUtils::convertJavaMapToQueryParamsMap(JNIEnv *env, jobject hashMap, OC::QueryParamsMap &map)
{
    if (!hashMap)
    {
        return;
    }

    jobject jEntrySet = env->CallObjectMethod(hashMap, g_mid_Map_entrySet);
    jobject jIterator = env->CallObjectMethod(jEntrySet, g_mid_Set_iterator);
    if (env->ExceptionCheck()) return;

    while (env->CallBooleanMethod(jIterator, g_mid_Iterator_hasNext))
    {
        jobject jEntry = env->CallObjectMethod(jIterator, g_mid_Iterator_next);
        jstring jKey = (jstring)env->CallObjectMethod(jEntry, g_mid_MapEntry_getKey);
        jstring jValue = (jstring)env->CallObjectMethod(jEntry, g_mid_MapEntry_getValue);

        map.insert(std::make_pair(
            env->GetStringUTFChars(jKey, NULL),
            env->GetStringUTFChars(jValue, NULL)
            ));
        if (env->ExceptionCheck()) return;
    }
}

jobject JniUtils::convertQueryParamsMapToJavaMap(JNIEnv *env, const OC::QueryParamsMap &map)
{
    jobject hashMap = env->NewObject(g_cls_HashMap, g_mid_HashMap_ctor);

    for (auto it = map.begin(); it != map.end(); ++it)
    {
        string key = it->first;
        string value = it->second;

        env->CallObjectMethod(hashMap,
            g_mid_HashMap_put,
            env->NewStringUTF(key.c_str()),
            env->NewStringUTF(value.c_str()));
        if (env->ExceptionCheck()) return nullptr;
    }

    return hashMap;
}

void JniUtils::convertJavaRepresentationArrToVector(JNIEnv *env,
    jobjectArray jRepresentationArray,
    std::vector<OC::OCRepresentation>& representationVector)
{
    if (!jRepresentationArray) return;

    jsize len = env->GetArrayLength(jRepresentationArray);

    for (jsize i = 0; i < len; ++i)
    {
        jobject jRep = env->GetObjectArrayElement(jRepresentationArray, i);
        OC::OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, jRep);
        representationVector.push_back(*rep);
        env->DeleteLocalRef(jRep);
        if (env->ExceptionCheck()) return;
    }
}

jobjectArray JniUtils::convertRepresentationVectorToJavaArray(JNIEnv *env,
    const std::vector<OC::OCRepresentation>& representationVector)
{
    jsize len = static_cast<jsize>(representationVector.size());
    jobjectArray repArr = env->NewObjectArray(len, g_cls_OcRepresentation, NULL);
    if (env->ExceptionCheck()) return nullptr;
    for (jsize i = 0; i < len; ++i)
    {
        OCRepresentation* rep = new OCRepresentation(representationVector[i]);
        jlong handle = reinterpret_cast<jlong>(rep);
        jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool, handle, true);

        env->SetObjectArrayElement(repArr, i, jRepresentation);
        env->DeleteLocalRef(jRepresentation);
        if (env->ExceptionCheck()) return nullptr;
    }

    return repArr;
}