struct Resource
{
  const void* data;
  size_t len;
};

class ResourceRepository
{
public:
  ResourceRepository();
public:
  virtual ~ResourceRepository();
public:
  const Resource& GetResource(const char* resourceId);
private:
  void addResource(const char* resourceId, const Resource& resource);
private:
  class impl;
  impl* pimpl;
};
