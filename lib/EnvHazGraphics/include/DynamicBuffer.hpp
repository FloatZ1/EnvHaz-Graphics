
#ifndef EHAZGRAPHICS_DYNAMIC_BUFFER_HPP
#define EHAZGRAPHICS_DYNAMIC_BUFFER_HPP

#include <glad/glad.h>
#include <cstdint>
#include "DataStructs.hpp"
#include <optional>
namespace eHazGraphics {


	class CDynamicBuffer
	{
	public:

		CDynamicBuffer();

		CDynamicBuffer(size_t p_szInitialSize, int p_iDynamicBufferID,
			GLenum p_gleTarget = GL_SHADER_STORAGE_BUFFER, bool p_bTrippleBuffer = true);


		
		template<typename T>
		SBufferRange InsertNewData(const T* p_pData, size_t p_szSize, TypeFlags p_tfType);



		SBufferRange InsertNewData(const void* p_pData, size_t p_szSize, TypeFlags p_tfType);

		void UpdateRange(SBufferRange* p_brRange, const void* p_pData, size_t p_szDataSize);

		void ResizeBuffer(size_t p_szMinimumSize = 1024UL);

		void ClearBuffer();


		std::optional<SAllocation> GetAllocation(int p_AllocationID);

		void SetBinding(int p_iBinding);
		void SetSlot(int p_Slot);
		void BindDynamicBuffer(TypeFlags type);



		uint32_t GetWriteSlot();
		int GetBufferID();




		void BeginWritting();

		void EndWritting();

		void Destroy();

	private:


		std::vector<SAllocation> m_Allocations;




		GLenum m_gleTarget = GL_SHADER_STORAGE_BUFFER;

		uint32_t m_uiDynamicBufferID;
		
		GLuint m_uiSlotIDs[3]{ 0,0,0 };
		void* m_pSlots[3]{ nullptr,nullptr,nullptr };
		size_t m_szBufferSize{0};
		size_t m_szWriteCursor[3] = {};
		size_t m_szOccupiedSize[3] = {};
		GLsync m_glsFences[3]{ 0,0,0 };
		
		int m_iBinding = 0;
		int m_iCurrentSlot = 0;
		int m_iNextSlot = 0;

		bool m_bSlotResizeState{false};

		bool m_bUseTrippleBuffering = true;

		uint64_t m_uiSlotTimeLine = 0;
		uint64_t m_uiSlotAge[3]{ 0,0,0 };


		uint32_t AllocateID(size_t p_szMinimumSize);
		
		SlotType GetDynamicSlotType(int p_ID);
		int GetDynamicSlotID(SlotType p_type);

		void SetDownFence(int p_Slot);
		void MapAllBufferSlots();
		bool WaitForSlotFence(int p_Slot);
		



};

}
#endif // !EHAZGRAPHICS_DYNAMIC_BUFFER_HPP
