#pragma once

#include <memory>

namespace nsK2EngineLow {
	template<class TResource>
	class TResourceBank : public Noncopyable {
	public:
		TResource* Get(const char* filePath)
		{
			auto it = m_resourceMap.find(filePath);
			if (it != m_resourceMap.end()) {
				//バンクに登録されている。
				return it->second.get();
			}
			return nullptr;
		}
		void Regist(const char* filePath, TResource* resource)
		{
			auto it = m_resourceMap.find(filePath);
			if (it == m_resourceMap.end()) {
				//未登録。
				m_resourceMap.insert(
					std::pair< std::string, TResourcePtr>(filePath, resource)
				);
			}
		}
		// Regist と異なり、未登録キーでも新規作成し、登録済みなら即座に置き換える
		// ホットリロードなど、初回登録と再登録を区別せず行う場合に使用する
		void Replace(const char* filePath, TResource* resource)
		{
			m_resourceMap[filePath].reset(resource);
		}
	private:
		using TResourcePtr = std::unique_ptr<TResource>;
		std::map<std::string, TResourcePtr> m_resourceMap;
	};
}