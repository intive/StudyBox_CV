#include "AzureStorageManager.h"


AzureStorageManager::AzureStorageManager()
{
    this->accountName = "devstoreaccount1";
    this->accountKey = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
    this->containerName = "fiszki";
}

AzureStorageManager::AzureStorageManager(std::string accountName, std::string containerName, std::string accountKey)
{
    this->accountName = accountName;
    this->containerName = containerName;
    this->accountKey = accountKey;
}


AzureStorageManager::~AzureStorageManager()
{
}

bool AzureStorageManager::downloadFromServer(std::string fileAddr)
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
    try
    {
        storageAccount = azure::storage::cloud_storage_account::parse(storageConnectionString);
        blobClient = storageAccount.create_cloud_blob_client();
        container = blobClient.get_container_reference(utility::conversions::to_string_t(this->containerName));
        if (!container.exists())
        {
            return false;
        }
    }
    catch(std::exception e)
    {
        return false;
    }

    //pobranie nazwy pliku z podanego parametru
    std::vector<std::string> elements;
    std::stringstream stream(fileAddr);
    std::string item;
    while (std::getline(stream, item, '/'))
        elements.push_back(item);

    azure::storage::cloud_block_blob blockBlob = container.get_blob_reference(utility::conversions::to_string_t(elements.back()));
    if(!blockBlob.exists())
    {
        return false;
    }

    //pobieranie obecnego czasu
    time_t currentTime;
    struct tm cTime;
    time(&currentTime);
    localtime_s(&cTime,&currentTime);
    std::string newFileName = std::to_string(cTime.tm_year)
        + std::to_string(cTime.tm_mon)
        + std::to_string(cTime.tm_mday)
        + std::to_string(cTime.tm_hour)
        + std::to_string(cTime.tm_hour)
        + std::to_string(cTime.tm_min)
        + std::to_string(cTime.tm_sec)
        + elements.back();

    //lokalny zapis pliku
    blockBlob.download_to_file(utility::conversions::to_string_t(newFileName));
    this->temporaryFileName = newFileName;
    return true;
}

std::string AzureStorageManager::uploadToServer(std::string path)
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
    try
    {
        storageAccount = azure::storage::cloud_storage_account::parse(storageConnectionString);
        blobClient = storageAccount.create_cloud_blob_client();
        container = blobClient.get_container_reference(utility::conversions::to_string_t(this->containerName));
        container.create_if_not_exists();
    }
   catch(const std::exception e)
   {
       return "Problem z polaczeniem";
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
        return "blob nie istnieje";
    }

    std::string returnString;
    returnString = "https://"+this->accountName+".blob.core.windows.net/"+this->containerName+"/"+elements.back();
    return returnString;
}

std::string AzureStorageManager::getTemporaryFileName()
{
    return this->temporaryFileName;
}
