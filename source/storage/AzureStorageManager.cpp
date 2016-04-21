#define _CRT_SECURE_NO_WARNINGS
#include "AzureStorageManager.h"
#include <ctime>
#include <fstream>

AzureStorageManager::AzureStorageManager()
{
    this->accountName = "devstoreaccount1";
    this->accountKey = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
    this->containerName = "fiszki";
}

AzureStorageManager::AzureStorageManager(const std::string& keyPath)
{
    std::ifstream file(keyPath, std::ios::binary);
    if (!file.is_open())
        throw AzureStorageError("file " + keyPath + "not found");
    file >> this->accountKey;
}

AzureStorageManager::AzureStorageManager(const std::string& accountName, const std::string& containerName, const std::string& accountKey)
    : accountName(accountName),
      containerName(containerName),
      accountKey(accountKey)
{

}


AzureStorageManager::~AzureStorageManager()
{
}

azure::storage::cloud_block_blob AzureStorageManager::prepareForDownload(const std::string& fileAddr) const
{
    auto err = "file of given address " + fileAddr + " not found";

    auto accountName = std::string((fileAddr[7]) == '/' ? fileAddr.begin() + 8 /* https */: fileAddr.begin() + 7 /* http */, fileAddr.begin() + fileAddr.find_first_of('.'));
    
    try
    {
        azure::storage::cloud_block_blob blockBlob(azure::storage::storage_uri(utility::conversions::to_string_t(fileAddr)), azure::storage::storage_credentials(
            utility::conversions::to_string_t(accountName), 
            utility::conversions::to_string_t(accountKey)));
        return blockBlob;
    }
    catch (const std::exception& e)
    {
        throw AzureStorageError(e.what());
    }
}

std::vector<unsigned char> AzureStorageManager::downloadToBuffer(const std::string& fileAddr)
{
    auto blockBlob = prepareForDownload(fileAddr);
    concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
    concurrency::streams::ostream output_stream(buffer);
    try
    {
        blockBlob.download_to_stream(output_stream);
    }
    catch (const std::exception& e)
    {
        throw AzureStorageError(e.what());
    }

    return buffer.collection();
}

std::string AzureStorageManager::downloadToString(const std::string& fileAddr)
{
    auto blockBlob = prepareForDownload(fileAddr);
    try
    {
        auto result = blockBlob.download_text();
        auto retval = utility::conversions::to_utf8string(result);
        return retval;
    }
    catch (const std::exception& e)
    {
        throw AzureStorageError(e.what());
    }
}

void AzureStorageManager::downloadToFile(const std::string& fileAddr, std::string destination)
{
    auto blockBlob = prepareForDownload(fileAddr);

    if (destination.empty())
    {
        //pobieranie obecnego czasu
        time_t currentTime;
        time(&currentTime);
        std::tm* cTime = std::localtime(&currentTime);
        destination = std::to_string(cTime->tm_year)
            + std::to_string(cTime->tm_mon)
            + std::to_string(cTime->tm_mday)
            + std::to_string(cTime->tm_hour)
            + std::to_string(cTime->tm_hour)
            + std::to_string(cTime->tm_min)
            + std::to_string(cTime->tm_sec);

        this->temporaryFileName = destination;

    }
    try
    {
        //lokalny zapis pliku
        blockBlob.download_to_file(utility::conversions::to_string_t(destination));
    }
    catch (const std::exception& e)
    {
        throw AzureStorageError(e.what());
    }
}

std::string AzureStorageManager::uploadToServer(const std::string& path)
{

    utility::string_t connectionString;
    /* connectionString = U("DefaultEndpointsProtocol=https;AccountName=")
    + utility::conversions::to_string_t(this->accountName)
    + utility::conversions::to_string_t(";AccountKey=")
    + utility::conversions::to_string_t(this->accountKey);*/
    connectionString = U("UseDevelopmentStorage=true;");
    utility::string_t storageConnectionString(connectionString);

    azure::storage::cloud_storage_account storageAccount;
    azure::storage::cloud_blob_client blobClient;
    azure::storage::cloud_blob_container container;
    std::string returnString;
    try
    {
        storageAccount = azure::storage::cloud_storage_account::parse(storageConnectionString);
        blobClient = storageAccount.create_cloud_blob_client();
        container = blobClient.get_container_reference(utility::conversions::to_string_t(this->containerName));
        container.create_if_not_exists();
    }
   catch(const std::exception e)
   {
       returnString = "Problem z polaczeniem";
       return returnString;
   }

    //właściwy upload
    //pobranie nazwy nowego bloba
    std::vector<std::string> elements;
    std::stringstream stream(path);
    std::string item;
    while (std::getline(stream, item, '/'))
        elements.push_back(item);

    std::string element;
    if(elements.size() == 0)
    {
        element = path;
    }
    else
    {
        element = elements.back();
    }

    azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(element));


    blockBlob.upload_from_file(utility::conversions::to_string_t(path));
    try
    {
        blockBlob.exists();
    }
    catch(std::exception e)
    {
        returnString = "blob nie istnieje";
        return returnString;
    }

    returnString = "https://"+this->accountName+".blob.core.windows.net/"+this->containerName+"/"+elements.back();
    return returnString;
}

std::string AzureStorageManager::getTemporaryFileName()
{
    return this->temporaryFileName;
}
