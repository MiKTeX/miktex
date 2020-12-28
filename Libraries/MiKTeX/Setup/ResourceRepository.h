class ResourceRepository
{
public:
  ResourceRepository();
public:
  virtual ~ResourceRepository();
public:
  const unsigned char* GetResource(const char* resourceId);
private:
  void addResource(const char* resourceId, const unsigned char* data);
private:
  class impl;
  impl* pimpl;
};
