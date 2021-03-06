#include <service/service_api.h>

#include <algorithm>
#include <sstream>
#include <unordered_set>

#include <toolbox/debug/debug.h>
#include <toolbox/utils/std_utils.h>


namespace {

///
/// \brief intersects will check if (s1 & s2) != 0
/// \param s1
/// \param s2
/// \return
///
template <typename T>
static bool
intersects(const std::set<T>& s1, const std::set<T>& s2)
{
  for (const T& e1 : s1) {
    if (s2.find(e1) != s2.end()) {
      return true;
    }
  }
  return false;
}

template <typename T>
static std::set<toolbox::UID>
convertToIDs(const std::set<T>& items)
{
  std::set<toolbox::UID> result;
  for (auto& item : items) {
    result.insert(item->id());
  }
  return result;
}


///
/// \brief Will normalize a tag text
/// \param text the text to normalize
/// \return the normalized tag text
///
static std::string
normalizeTagText(const std::string& text)
{
  // TODO: here will be good if we normalize also special characters or
  // other things.
  std::string result = text;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  result.erase(std::remove_if(result.begin(), result.end(), [](char c){return std::isspace(c);}), result.end());

  return result;
}

/**
 * @brief Gather all the tags being used by a list of contnets
 * @param contents The list of contents
 * @return the list of tag ids being used by the provided contents
 */
static std::unordered_set<toolbox::UID>
getAllTagIDs(const std::vector<data::Content::Ptr>& contents)
{
  std::unordered_set<toolbox::UID> result;
  for (auto& content : contents) {
    for (auto& tag_id : content->tagIDs()) {
      result.insert(tag_id);
    }
  }
  return result;
}

}

namespace service {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


std::set<toolbox::UID>
ServiceAPI::getCommonContentIDsFromTags(const std::vector<data::Tag::ConstPtr>& tags) const
{
  std::set<toolbox::UID> result;
  for (const data::Tag::ConstPtr& curr_tag : tags) {
    const std::set<toolbox::UID> content_ids = convertToIDs(data_mapper_->contentsForTag(curr_tag->id()));
    if (result.empty()) {
      result = content_ids;
    } else {
      std::set<toolbox::UID> tmp;
      std::set_intersection(result.begin(),
                            result.end(),
                            content_ids.begin(),
                            content_ids.end(),
                            std::inserter(tmp, tmp.begin()));
      result = tmp;
    }
  }

  return result;
}

std::set<data::Tag::ConstPtr>
ServiceAPI::getRelevantSuggestions(const std::string& query,
                                   const std::set<data::Tag::ConstPtr>& current_tags,
                                   const std::set<toolbox::UID>& common_elements) const
{
  std::set<data::Tag::ConstPtr> result;

  const std::string norm_query = normalizeTagText(query);
  auto suggestions = data_mapper_->suggestedTags(norm_query);
  for (const auto& suggested_tag : suggestions) {
    // here we will filtered out the Tags that don't have any element in common
    // with the Tags already set by the user
    if (current_tags.empty() ||
       (current_tags.find(suggested_tag) == current_tags.end() &&
       intersects(convertToIDs(data_mapper_->contentsForTag(suggested_tag->id())), common_elements))) {
      result.insert(suggested_tag);
    }

    if (config_.max_number_tag_results > 0 &&
        int(result.size()) >= config_.max_number_tag_results) {
      break;
    }
  }

  return result;
}

std::set<data::Content::ConstPtr>
ServiceAPI::getContents(const std::set<toolbox::UID>& ids) const
{
  std::set<data::Content::ConstPtr> result;
  for (const toolbox::UID& id : ids) {
    result.insert(data_mapper_->contentFromID(id));
  }
  return result;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ServiceAPI::ServiceAPI(data::DataMapper::Ptr data_mapper,
                       storage::DataStorage::Ptr data_storage,
                       const Config& config) :
  data_mapper_(data_mapper)
, data_storage_(data_storage)
, config_(config)
{
}

////////////////////////////////////////////////////////////////////////////////
ServiceAPI::~ServiceAPI()
{

}

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

bool
ServiceAPI::getTagByName(const std::string& name, data::Tag::ConstPtr& result) const
{
  result = data_mapper_->tagFromName(name);
  return result.get() != nullptr;
}

bool
ServiceAPI::getTagsByIds(const std::vector<toolbox::UID>& tag_ids,
                         std::vector<data::Tag::ConstPtr>& tags) const
{
  bool result = true;
  tags.clear();
  tags.reserve(tag_ids.size());
  for (auto& tag_id : tag_ids) {
    data::Tag::ConstPtr tag = data_mapper_->tagFromID(tag_id);
    if (tag.get() == nullptr) {
      LOG_INFO("There is no tag with ID " << tag_id);
      result = false;
    } else {
      tags.push_back(tag);
    }
  }

  return result;
}

bool
ServiceAPI::getContentById(const toolbox::UID& content_id, data::Content::ConstPtr& content) const
{
  content = data_mapper_->contentFromID(content_id);
  return content.get() != nullptr;
}

bool
ServiceAPI::hasContentWithId(const toolbox::UID& content_id) const
{
  data::Content::ConstPtr dummy;
  return getContentById(content_id, dummy);
}

bool
ServiceAPI::searchTags(const SearchContext& context, TagSearchReslut& result) const
{
  result.expanded_tags.clear();

  if (context.query.empty()) {
    return true;
  }

  const std::set<toolbox::UID> common_content_ids = getCommonContentIDsFromTags(context.tags);
  const std::set<data::Tag::ConstPtr> tag_set = toolbox::StdUtils::vectorToSet(context.tags);
  result.expanded_tags = getRelevantSuggestions(context.query, tag_set, common_content_ids);

  return true;
}

bool
ServiceAPI::searchContent(const SearchContext& context, ContentSearchResult& result) const
{
  result.exp_results.clear();
  result.tagged_contents.clear();

  const std::set<toolbox::UID> common_content_ids = getCommonContentIDsFromTags(context.tags);
  const std::set<data::Tag::ConstPtr> tag_set = toolbox::StdUtils::vectorToSet(context.tags);
  std::set<data::Tag::ConstPtr> expanded_tags =
      getRelevantSuggestions(context.query, tag_set, common_content_ids);

  // now we have the associated elements for all the current Tags
  result.tagged_contents = getContents(common_content_ids);
  for (const data::Tag::ConstPtr& exp_tag : expanded_tags) {
    // get the intersection for this case if and only if there are some
    // Tags already set
    std::set<toolbox::UID> tmp;
    const std::set<toolbox::UID> content_ids = convertToIDs(data_mapper_->contentsForTag(exp_tag->id()));
    if (!context.tags.empty()) {
      std::set_intersection(common_content_ids.begin(),
                            common_content_ids.end(),
                            content_ids.begin(),
                            content_ids.end(),
                            std::inserter(tmp, tmp.begin()));
    } else {
      tmp = content_ids;
    }
    result.exp_results[exp_tag] = getContents(tmp);
  }

  return true;
}

data::Tag::ConstPtr
ServiceAPI::createTag(const std::string& name)
{
  data::Tag::Ptr tag(new data::Tag(name));

  if (!data_storage_->saveTag(tag)) {
    LOG_ERROR("we couldn't save the tag " << *tag);
    return nullptr;
  }

  data_mapper_->addTag(tag);

  return tag;
}

data::Tag::ConstPtr
ServiceAPI::updateTag(const toolbox::UID& tag_id, data::Tag::Ptr tag_data)
{
  if (tag_data.get() == nullptr) {
    LOG_INFO("Tag is null");
    return nullptr;
  }

  if (!data_mapper_->hasTag(tag_id)) {
    LOG_INFO("cannot update the tag with id " << tag_id);
    return nullptr;
  }
  auto tag_ptr = data_mapper_->tagFromID(tag_id);
  tag_ptr->setName(tag_data->name());

  if (!data_storage_->saveTag(tag_ptr)) {
    LOG_ERROR("we couldn't save the tag " << *tag_ptr);
    return nullptr;
  }

  return tag_ptr;
}

bool
ServiceAPI::deleteTag(const toolbox::UID& tag_id)
{
  if (!data_mapper_->hasTag(tag_id)) {
    LOG_INFO("cannot delete the tag with id " << tag_id);
    return false;
  }

  // TODO: update storage here
  ASSERT(false && "we need to implement this");
  // TODO: update all contents that have this tag. Probably we do not want to do this

  return true;
}

data::Content::Ptr
ServiceAPI::createContent(data::ContentType meta_type,
                          bool meta_encrypted,
                          const std::string& data,
                          const std::set<toolbox::UID>& tag_ids)
{
  // check if we have all the tags here
  std::set<data::Tag::Ptr> tags = data_mapper_->tagsFromIDs(tag_ids);

  if (tags.size() != tag_ids.size()) {
    LOG_INFO("We couldn't find all the tags... something is wrong here");
    return nullptr;
  }

  data::Metadata metadata;
  metadata.setType(meta_type);
  metadata.setEncrypted(meta_encrypted);
  // TODO: we should encrypt this if needed.
  data::Content::Ptr result(new data::Content());
  result->setData(data);
  result->setTagIDs(tag_ids);
  result->setMetadata(metadata);

  if (!data_storage_->saveContent(result)) {
    LOG_ERROR("Error saving the content with id " << result->id());
    return nullptr;
  }

  data_mapper_->addContent(result);

  return result;
}

data::Content::Ptr
ServiceAPI::updateContent(const toolbox::UID& content_id, data::Content::Ptr content)
{
  if (content.get() == nullptr) {
    LOG_INFO("Content is null");
    return nullptr;
  }

  if (!data_mapper_->hasContent(content_id)) {
    LOG_INFO("There is no content with id " << content_id);
    return nullptr;
  }

  data::Content::Ptr content_ptr = data_mapper_->contentFromID(content_id);
  content_ptr->copyFrom(*content);

  if (!data_storage_->saveContent(content_ptr)) {
    LOG_ERROR("Error saving the content with id " << content_ptr->id());
    return nullptr;
  }

  // TODO: we may want to add an update method on the DataMapper
  data_mapper_->removeContent(content_ptr);
  data_mapper_->addContent(content_ptr);


  return content_ptr;
}

bool
ServiceAPI::deleteContent(const toolbox::UID& content_id)
{
  if (!data_mapper_->hasContent(content_id)) {
    LOG_INFO("There is no content with id " << content_id);
    return false;
  }

  data::Content::Ptr content_ptr = data_mapper_->contentFromID(content_id);
  data_mapper_->removeContent(content_ptr);

  if (!data_storage_->removeContent(content_ptr)) {
    LOG_ERROR("Error deleting the content with id " << content_id);
    return false;
  }

  return true;
}

bool
ServiceAPI::performTagCleanup(const TagCleanup& options)
{
  // get all tags that are associated to a content first
  const std::unordered_set<toolbox::UID> used_tags = getAllTagIDs(data_mapper_->allContents());
  const std::vector<data::Tag::Ptr> all_tags = data_mapper_->allTags();
  std::unordered_set<data::Tag::Ptr> unused_tags;
  for (auto& tag : all_tags) {
    if (used_tags.count(tag->id()) <= 0) {
      unused_tags.insert(tag);
    }
  }

  // now we remove all the tags that are not being used
  for (auto& unused_tag : unused_tags) {
    data_mapper_->removeTag(unused_tag);
    LOG_INFO("Removing unused tag: " << unused_tag->name());
    if (options.storage_delete_unused) {
      data_storage_->removeTag(unused_tag);
    }
  }

  LOG_INFO("Removed " << unused_tags.size() << " unused tags");

  return true;
}



} // namespace service
