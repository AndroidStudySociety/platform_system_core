//
// Copyright (C) 2019 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#if !defined(__ANDROID_RECOVERY__)
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/gsi/IGsiService.h>
#include <android/gsi/IGsid.h>
#include <binder/IServiceManager.h>
#include <libfiemap/image_manager.h>
#include <libgsi/libgsi.h>

namespace android {
namespace fiemap {

using namespace android::gsi;
using namespace std::chrono_literals;

class ImageManagerBinder final : public IImageManager {
  public:
    ImageManagerBinder(android::sp<IGsiService>&& service, android::sp<IImageService>&& manager);
    bool CreateBackingImage(const std::string& name, uint64_t size, int flags) override;
    bool DeleteBackingImage(const std::string& name) override;
    bool MapImageDevice(const std::string& name, const std::chrono::milliseconds& timeout_ms,
                        std::string* path) override;
    bool UnmapImageDevice(const std::string& name) override;
    bool BackingImageExists(const std::string& name) override;
    bool IsImageMapped(const std::string& name) override;
    bool MapImageWithDeviceMapper(const IPartitionOpener& opener, const std::string& name,
                                  std::string* dev) override;
    bool ZeroFillNewImage(const std::string& name, uint64_t bytes) override;
    bool RemoveAllImages() override;
    bool DisableImage(const std::string& name) override;
    bool RemoveDisabledImages() override;
    bool GetMappedImageDevice(const std::string& name, std::string* device) override;
    bool MapAllImages(const std::function<bool(std::set<std::string>)>& init) override;

    std::vector<std::string> GetAllBackingImages() override;

  private:
    android::sp<IGsiService> service_;
    android::sp<IImageService> manager_;
};

ImageManagerBinder::ImageManagerBinder(android::sp<IGsiService>&& service,
                                       android::sp<IImageService>&& manager)
    : service_(std::move(service)), manager_(std::move(manager)) {}

bool ImageManagerBinder::CreateBackingImage(const std::string& name, uint64_t size, int flags) {
    auto status = manager_->createBackingImage(name, size, flags);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::DeleteBackingImage(const std::string& name) {
    auto status = manager_->deleteBackingImage(name);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::MapImageDevice(const std::string& name,
                                        const std::chrono::milliseconds& timeout_ms,
                                        std::string* path) {
    int32_t timeout_ms_count =
            static_cast<int32_t>(std::clamp<typename std::chrono::milliseconds::rep>(
                    timeout_ms.count(), INT32_MIN, INT32_MAX));
    MappedImage map;
    auto status = manager_->mapImageDevice(name, timeout_ms_count, &map);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    *path = map.path;
    return true;
}

bool ImageManagerBinder::UnmapImageDevice(const std::string& name) {
    auto status = manager_->unmapImageDevice(name);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::BackingImageExists(const std::string& name) {
    bool retval;
    auto status = manager_->backingImageExists(name, &retval);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return retval;
}

bool ImageManagerBinder::IsImageMapped(const std::string& name) {
    bool retval;
    auto status = manager_->isImageMapped(name, &retval);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return retval;
}

bool ImageManagerBinder::MapImageWithDeviceMapper(const IPartitionOpener& opener,
                                                  const std::string& name, std::string* dev) {
    (void)opener;
    (void)name;
    (void)dev;
    LOG(ERROR) << "MapImageWithDeviceMapper is not available over binder.";
    return false;
}

std::vector<std::string> ImageManagerBinder::GetAllBackingImages() {
    std::vector<std::string> retval;
    auto status = manager_->getAllBackingImages(&retval);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
    }
    return retval;
}

bool ImageManagerBinder::ZeroFillNewImage(const std::string& name, uint64_t bytes) {
    auto status = manager_->zeroFillNewImage(name, bytes);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::RemoveAllImages() {
    auto status = manager_->removeAllImages();
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::DisableImage(const std::string&) {
    LOG(ERROR) << __PRETTY_FUNCTION__ << " is not available over binder";
    return false;
}

bool ImageManagerBinder::RemoveDisabledImages() {
    auto status = manager_->removeDisabledImages();
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return true;
}

bool ImageManagerBinder::GetMappedImageDevice(const std::string& name, std::string* device) {
    auto status = manager_->getMappedImageDevice(name, device);
    if (!status.isOk()) {
        LOG(ERROR) << __PRETTY_FUNCTION__
                   << " binder returned: " << status.exceptionMessage().string();
        return false;
    }
    return !device->empty();
}

bool ImageManagerBinder::MapAllImages(const std::function<bool(std::set<std::string>)>&) {
    LOG(ERROR) << __PRETTY_FUNCTION__ << " not available over binder";
    return false;
}

static android::sp<IGsid> AcquireIGsid(const std::chrono::milliseconds& timeout_ms) {
    if (android::base::GetProperty("init.svc.gsid", "") != "running") {
        if (!android::base::SetProperty("ctl.start", "gsid") ||
            !android::base::WaitForProperty("init.svc.gsid", "running", timeout_ms)) {
            LOG(ERROR) << "Could not start the gsid service";
            return nullptr;
        }
        // Sleep for 250ms to give the service time to register.
        usleep(250 * 1000);
    }
    auto sm = android::defaultServiceManager();
    auto name = android::String16(kGsiServiceName);
    auto service = sm->checkService(name);
    return android::interface_cast<IGsid>(service);
}

static android::sp<IGsid> GetGsiService(const std::chrono::milliseconds& timeout_ms) {
    auto start_time = std::chrono::steady_clock::now();

    std::chrono::milliseconds elapsed = std::chrono::milliseconds::zero();
    do {
        if (auto gsid = AcquireIGsid(timeout_ms - elapsed); gsid != nullptr) {
            return gsid;
        }
        auto now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    } while (elapsed <= timeout_ms);

    LOG(ERROR) << "Timed out trying to acquire IGsid interface";
    return nullptr;
}

std::unique_ptr<IImageManager> IImageManager::Open(const std::string& dir,
                                                   const std::chrono::milliseconds& timeout_ms) {
    auto gsid = GetGsiService(timeout_ms);
    if (!gsid) {
        return nullptr;
    }

    android::sp<IGsiService> service;
    auto status = gsid->getClient(&service);
    if (!status.isOk() || !service) {
        LOG(ERROR) << "Could not acquire IGsiService";
        return nullptr;
    }

    android::sp<IImageService> manager;
    status = service->openImageService(dir, &manager);
    if (!status.isOk() || !manager) {
        LOG(ERROR) << "Could not acquire IImageManager: " << status.exceptionMessage().string();
        return nullptr;
    }
    return std::make_unique<ImageManagerBinder>(std::move(service), std::move(manager));
}

}  // namespace fiemap
}  // namespace android

#endif  // __ANDROID_RECOVERY__
