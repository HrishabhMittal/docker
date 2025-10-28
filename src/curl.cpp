#pragma once
#include <curl/curl.h>
#include <string>
#include <archive.h>
#include <archive_entry.h>
#include "JASON/src/parser.cpp"
std::string curl(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string response_data;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl=curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,
            +[](void *contents,size_t size,size_t nmemb,std::string *userp) {
                size_t total_size=size * nmemb;
                userp->append((char*)contents,total_size);
                return total_size;
            });
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,&response_data);
        res=curl_easy_perform(curl);
        if(res!=CURLE_OK) {
            fprintf(stderr,"curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response_data;
}
std::string curl(const std::string& url,const std::string& header) {
    CURL *curl;
    CURLcode res;
    std::string response_data;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl=curl_easy_init();
    if(curl) {
        struct curl_slist *chunk=NULL;
        chunk=curl_slist_append(chunk,header.c_str());
        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,chunk);
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,
            +[](void *contents,size_t size,size_t nmemb,std::string *userp) {
                size_t total_size=size * nmemb;
                userp->append((char*)contents,total_size);
                return total_size;
            });
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,&response_data);
        res=curl_easy_perform(curl);
        if(res!=CURLE_OK) {
            fprintf(stderr,"curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        }
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response_data;
}
std::string authURL(const std::string& repository,const std::string& image) {
    return "https://auth.docker.io/token?service=registry.docker.io&scope=repository:"+repository+"/"+image+":pull";
}
std::string registryURL(const std::string& repository,const std::string& image,const std::string& tag) {
    return "https://registry-1.docker.io/v2/"+repository+"/"+image+"/manifests/"+tag;
}
std::string getAuthToken(const std::string& repository,const std::string& image) {
    std::string url=authURL(repository,image);
    std::string response=curl(url);
    JsonParser parser(response);
    auto json=parser.ParseJSON();
    return json.obj["token"].str;
}
std::string ImageManifestURL(const std::string& repository,const std::string& image,const std::string& tag,const std::string& token) {
    return "https://registry.hub.docker.com/v2/"+repository+"/"+image+"/manifests/"+tag;
}

std::string createAuthHeader(const std::string& token) {
    return "Authorization: Bearer "+token;
}
std::string ImageLayerURL(const std::string& repository,const std::string& image,const std::string& digest) {
    return "https://registry-1.docker.io/v2/"+repository+"/"+image+"/blobs/"+digest;
}

int PullImageLayer(const std::string& repository,const std::string& image,const std::string& digest,const std::string& token,const std::string& dir) {
    std::string url=ImageLayerURL(repository,image,digest);
    std::string header=createAuthHeader(token);
    std::string response=curl(url,header);
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    a=archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);
    ext=archive_write_disk_new();
    archive_write_disk_set_options(ext,ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);
    archive_read_open_memory(a,response.data(),response.size());
    while (archive_read_next_header(a,&entry) == ARCHIVE_OK) {
        const char* current_file=archive_entry_pathname(entry);
        std::string full_output_path=dir+"/"+current_file;
        archive_entry_set_pathname(entry,full_output_path.c_str());
        archive_write_header(ext,entry);
        const void* buff;
        size_t size;
        la_int64_t offset;
        while (archive_read_data_block(a,&buff,&size,&offset) == ARCHIVE_OK) {
            archive_write_data_block(ext,buff,size,offset);
        }
        archive_write_finish_entry(ext);
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return 0;
}
const std::string manifestV2MediaType="application/vnd.docker.distribution.manifest.v2+json";
std::string PullImageLayers(const std::string& repository,const std::string& image,const std::string& tag,const std::string& token,const std::string& dir) {
    std::string url=ImageManifestURL(repository,image,tag,token);
    std::string header=manifestV2MediaType+createAuthHeader(token);
    std::string response=curl(url,header);
    JsonParser parser(response);
    auto json=parser.ParseJSON();
    for (auto& layer:json.obj["layers"].arr) {
        std::string digest=layer.obj["digest"].str;
        PullImageLayer(repository,image,digest,token,dir);
    }
    return "Image layers pulled successfully.";
}

std::string PullDockerImage(const std::string& full_image_name,const std::string& dir) {
    const std::string defaultRepositoryName="library";
    const std::string defaultTag="latest";

    size_t repository_end=full_image_name.find('/');
    std::string repository=(repository_end!=std::string::npos)?full_image_name.substr(0,repository_end):defaultRepositoryName;
    size_t tag_start=full_image_name.rfind(':');
    std::string tag=(tag_start!=std::string::npos)?full_image_name.substr(tag_start+1):defaultTag;
    size_t image_start=(repository_end!=std::string::npos)?repository_end+1:0;
    std::string image=(tag_start!=std::string::npos)?full_image_name.substr(image_start,tag_start-image_start):full_image_name.substr(image_start);

    std::string token=getAuthToken(repository,image);
    return PullImageLayers(repository,image,tag,token,dir);
}