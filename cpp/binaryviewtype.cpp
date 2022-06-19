// Copyright (c) 2015-2022 Vector 35 Inc
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "core/binaryview.h"
#include "core/platform.h"
#include "binaryviewtype.hpp"
#include "settings.hpp"
#include "metadata.hpp"
#include "getobject.hpp"

using namespace BinaryNinja;
using namespace std;


BNBinaryView* BinaryViewType::CreateCallback(void* ctxt, BNBinaryView* data)
{
	BinaryViewType* type = (BinaryViewType*)ctxt;
	Ref<BinaryView> view = CreateNewView(data);
	Ref<BinaryView> result = type->Create(view);
	if (!result)
		return nullptr;
	return BNNewViewReference(BinaryNinja::GetObject(result));
}


BNBinaryView* BinaryViewType::ParseCallback(void* ctxt, BNBinaryView* data)
{
	BinaryViewType* type = (BinaryViewType*)ctxt;
	Ref<BinaryView> view = CreateNewView(data);
	Ref<BinaryView> result = type->Parse(view);
	if (!result)
		return nullptr;
	return BNNewViewReference(BinaryNinja::GetObject(result));
}


bool BinaryViewType::IsValidCallback(void* ctxt, BNBinaryView* data)
{
	BinaryViewType* type = (BinaryViewType*)ctxt;
	Ref<BinaryView> view = CreateNewView(data);
	return type->IsTypeValidForData(view);
}


bool BinaryViewType::IsDeprecatedCallback(void* ctxt)
{
	BinaryViewType* type = (BinaryViewType*)ctxt;
	return type->IsDeprecated();
}


BNSettings* BinaryViewType::GetSettingsCallback(void* ctxt, BNBinaryView* data)
{
	BinaryViewType* type = (BinaryViewType*)ctxt;
	Ref<BinaryView> view = CreateNewView(data);
	Ref<Settings> result = type->GetLoadSettingsForData(view);
	if (!result)
		return nullptr;
	return BNNewSettingsReference(result->GetObject());
}


BinaryViewType::BinaryViewType(BNBinaryViewType* type)
{
	m_object = type;
}


BinaryViewType::BinaryViewType(const string& name, const string& longName) :
    m_nameForRegister(name), m_longNameForRegister(longName)
{
	m_object = nullptr;
}


void BinaryViewType::Register(BinaryViewType* type)
{
	BNCustomBinaryViewType callbacks;
	callbacks.context = type;
	callbacks.create = CreateCallback;
	callbacks.parse = ParseCallback;
	callbacks.isValidForData = IsValidCallback;
	callbacks.isDeprecated = IsDeprecatedCallback;
	callbacks.getLoadSettingsForData = GetSettingsCallback;

	type->AddRefForRegistration();
	type->m_object =
	    BNRegisterBinaryViewType(type->m_nameForRegister.c_str(), type->m_longNameForRegister.c_str(), &callbacks);
}


Ref<BinaryViewType> BinaryViewType::GetByName(const string& name)
{
	BNBinaryViewType* type = BNGetBinaryViewTypeByName(name.c_str());
	if (!type)
		return nullptr;
	return new CoreBinaryViewType(type);
}


vector<Ref<BinaryViewType>> BinaryViewType::GetViewTypes()
{
	BNBinaryViewType** types;
	size_t count;
	types = BNGetBinaryViewTypes(&count);

	vector<Ref<BinaryViewType>> result;
	result.reserve(count);
	for (size_t i = 0; i < count; i++)
		result.push_back(new CoreBinaryViewType(types[i]));

	BNFreeBinaryViewTypeList(types);
	return result;
}


vector<Ref<BinaryViewType>> BinaryViewType::GetViewTypesForData(BinaryView* data)
{
	BNBinaryViewType** types;
	size_t count;
	types = BNGetBinaryViewTypesForData(BinaryNinja::GetObject(data), &count);

	vector<Ref<BinaryViewType>> result;
	result.reserve(count);
	for (size_t i = 0; i < count; i++)
		result.push_back(new CoreBinaryViewType(types[i]));

	BNFreeBinaryViewTypeList(types);
	return result;
}


void BinaryViewType::RegisterArchitecture(const string& name, uint32_t id, BNEndianness endian, Architecture* arch)
{
	Ref<BinaryViewType> type = BinaryViewType::GetByName(name);
	if (!type)
		return;
	type->RegisterArchitecture(id, endian, arch);
}


void BinaryViewType::RegisterArchitecture(uint32_t id, BNEndianness endian, Architecture* arch)
{
	BNRegisterArchitectureForViewType(m_object, id, endian, BinaryNinja::GetObject(arch));
}


Ref<Architecture> BinaryViewType::GetArchitecture(uint32_t id, BNEndianness endian)
{
	BNArchitecture* arch = BNGetArchitectureForViewType(m_object, id, endian);
	if (!arch)
		return nullptr;
	return CreateNewCoreArchitecture(arch);
}


void BinaryViewType::RegisterPlatform(const string& name, uint32_t id, Architecture* arch, Platform* platform)
{
	Ref<BinaryViewType> type = BinaryViewType::GetByName(name);
	if (!type)
		return;
	type->RegisterPlatform(id, arch, platform);
}


void BinaryViewType::RegisterDefaultPlatform(const string& name, Architecture* arch, Platform* platform)
{
	Ref<BinaryViewType> type = BinaryViewType::GetByName(name);
	if (!type)
		return;
	type->RegisterDefaultPlatform(arch, platform);
}


void BinaryViewType::RegisterPlatform(uint32_t id, Architecture* arch, Platform* platform)
{
	BNRegisterPlatformForViewType(m_object, id, BinaryNinja::GetObject(arch), BinaryNinja::GetObject(platform));
}


void BinaryViewType::RegisterDefaultPlatform(Architecture* arch, Platform* platform)
{
	BNRegisterDefaultPlatformForViewType(m_object, BinaryNinja::GetObject(arch), BinaryNinja::GetObject(platform));
}


Ref<Platform> BinaryViewType::GetPlatform(uint32_t id, Architecture* arch)
{
	BNPlatform* platform = BNGetPlatformForViewType(m_object, id, BinaryNinja::GetObject(arch));
	if (!platform)
		return nullptr;
	return CreateNewPlatform(platform);
}


void BinaryViewType::RegisterPlatformRecognizer(uint64_t id, BNEndianness endian,
    const std::function<Ref<Platform>(BinaryView* view, Metadata* metadata)>& callback)
{
	PlatformRecognizerFunction* ctxt = new PlatformRecognizerFunction;
	ctxt->action = callback;
	BNRegisterPlatformRecognizerForViewType(m_object, id, endian, PlatformRecognizerCallback, ctxt);
}


Ref<Platform> BinaryViewType::RecognizePlatform(uint64_t id, BNEndianness endian, BinaryView* view, Metadata* metadata)
{
	BNPlatform* platform =
	    BNRecognizePlatformForViewType(m_object, id, endian, BinaryNinja::GetObject(view), metadata->GetObject());
	if (!platform)
		return nullptr;
	return CreateNewPlatform(platform);
}


string BinaryViewType::GetName()
{
	char* contents = BNGetBinaryViewTypeName(m_object);
	string result = contents;
	BNFreeString(contents);
	return result;
}


string BinaryViewType::GetLongName()
{
	char* contents = BNGetBinaryViewTypeLongName(m_object);
	string result = contents;
	BNFreeString(contents);
	return result;
}

bool BinaryViewType::IsDeprecated()
{
	return BNIsBinaryViewTypeDeprecated(m_object);
}


void BinaryViewType::RegisterBinaryViewFinalizationEvent(const function<void(BinaryView* view)>& callback)
{
	BinaryViewEvent* event = new BinaryViewEvent;
	event->action = callback;
	BNRegisterBinaryViewEvent(BinaryViewFinalizationEvent, BinaryViewEventCallback, event);
}


void BinaryViewType::RegisterBinaryViewInitialAnalysisCompletionEvent(const function<void(BinaryView* view)>& callback)
{
	BinaryViewEvent* event = new BinaryViewEvent;
	event->action = callback;
	BNRegisterBinaryViewEvent(BinaryViewInitialAnalysisCompletionEvent, BinaryViewEventCallback, event);
}


void BinaryViewType::BinaryViewEventCallback(void* ctxt, BNBinaryView* view)
{
	BinaryViewEvent* event = (BinaryViewEvent*)ctxt;
	Ref<BinaryView> viewObject = CreateNewReferencedView(view);
	event->action(viewObject);
}


BNPlatform* BinaryViewType::PlatformRecognizerCallback(void* ctxt, BNBinaryView* view, BNMetadata* metadata)
{
	PlatformRecognizerFunction* callback = (PlatformRecognizerFunction*)ctxt;
	Ref<BinaryView> viewObject = CreateNewReferencedView(view);
	Ref<Metadata> metadataObject = new Metadata(BNNewMetadataReference(metadata));
	Ref<Platform> result = callback->action(viewObject, metadataObject);
	if (!result)
		return nullptr;
	return BNNewPlatformReference(BinaryNinja::GetObject(result));
}


CoreBinaryViewType::CoreBinaryViewType(BNBinaryViewType* type) : BinaryViewType(type) {}


BinaryView* CoreBinaryViewType::Create(BinaryView* data)
{
	BNBinaryView* view = BNCreateBinaryViewOfType(m_object, BinaryNinja::GetObject(data));
	return CreateNewView(view);
}


BinaryView* CoreBinaryViewType::Parse(BinaryView* data)
{
	BNBinaryView* view = BNParseBinaryViewOfType(m_object, BinaryNinja::GetObject(data));
	return CreateNewView(view);
}


bool CoreBinaryViewType::IsTypeValidForData(BinaryView* data)
{
	return BNIsBinaryViewTypeValidForData(m_object, BinaryNinja::GetObject(data));
}


Ref<Settings> CoreBinaryViewType::GetLoadSettingsForData(BinaryView* data)
{
	BNSettings* settings = BNGetBinaryViewLoadSettingsForData(m_object, BinaryNinja::GetObject(data));
	if (!settings)
		return nullptr;
	return new Settings(settings);
}