#include "DynamicBuffer.hpp"




namespace eHazGraphics {







	CDynamicBuffer::CDynamicBuffer()
	{
	}

	CDynamicBuffer::CDynamicBuffer(size_t p_szInitialSize, int p_iDynamicBufferID, GLenum p_gleTarget, bool p_bTrippleBuffer)
	{
		m_szBufferSize = p_szInitialSize;
		m_uiDynamicBufferID = p_iDynamicBufferID;
		m_gleTarget = p_gleTarget;
		m_bUseTrippleBuffering = p_bTrippleBuffer;


		if (m_bUseTrippleBuffering) {

			glCreateBuffers(1, &m_uiSlotIDs[0]);
			glCreateBuffers(1, &m_uiSlotIDs[1]);
			glCreateBuffers(1, &m_uiSlotIDs[2]);

			glNamedBufferStorage(m_uiSlotIDs[0], p_szInitialSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
				GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
			glNamedBufferStorage(m_uiSlotIDs[1], p_szInitialSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
				GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
			glNamedBufferStorage(m_uiSlotIDs[2], p_szInitialSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
				GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		}
		else {
			glCreateBuffers(1, &m_uiSlotIDs[0]);
			glNamedBufferStorage(m_uiSlotIDs[0], p_szInitialSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
				GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

		}
		
		MapAllBufferSlots();


#ifdef EHAZ_DEBUG
		GLint result = 0;
		glGetNamedBufferParameteriv(m_uiSlotIDs[0], GL_BUFFER_SIZE, &result);
		SDL_Log("Buffer %u size = %d", m_uiSlotIDs[0], result);

		GLint result1 = 0;
		glGetNamedBufferParameteriv(m_uiSlotIDs[1], GL_BUFFER_SIZE, &result1);
		SDL_Log("Buffer %u size = %d", m_uiSlotIDs[1], result1);

		GLint result2 = 0;
		glGetNamedBufferParameteriv(m_uiSlotIDs[2], GL_BUFFER_SIZE, &result2);
		SDL_Log("Buffer %u size = %d", m_uiSlotIDs[2], result2);
#endif

	}
	template<typename T>
	SBufferRange CDynamicBuffer::InsertNewData(const T* p_pData, size_t p_szSize, TypeFlags p_tfType)
	{

		if (std::is_same<T, void>::value == true) {
			return InsertNewData(p_pData, p_szSize, p_tfType);
		}



		if (p_szSize >= m_szBufferSize || p_szSize + m_szOccupiedSize[GetWriteSlot()] >= m_szBufferSize) {
			m_bSlotResizeState = true;
			ResizeBuffer(p_szSize);
		}


		int slot = GetWriteSlot();
		
		std::byte* base = static_cast<std::byte*>(m_pSlots[slot]);
		T* l_pWriteLocation = reinterpret_cast<T*>(base + m_szWriteCursor[slot]);

		uint32_t count = 1;

		std::memcpy(l_pWriteLocation, p_pData, p_szSize);
		count = p_szSize / sizeof(T);

		uint32_t allocID = AllocateID(p_szSize);
		SAllocation& l_allocation = m_Allocations[allocID];


		l_allocation.alive = true;
		l_allocation.generation++;
		l_allocation.offset = m_szWriteCursor[slot];
		l_allocation.size = p_szSize;


		SBufferHandle handle;
		handle.allocationID = allocID;
		handle.bufferID = m_uiDynamicBufferID;
		handle.slot = GetDynamicSlotType(slot);
		handle.generation = l_allocation.generation;

		m_szOccupiedSize[slot] += p_szSize;

		SBufferRange l_Range;
		l_Range.count = count;
		l_Range.dataType = p_tfType;
		l_Range.handle = handle;

		m_szWriteCursor[slot] += p_szSize;
		m_szOccupiedSize[slot] += p_szSize;



		return l_Range;




	}

	SBufferRange CDynamicBuffer::InsertNewData(const void* p_pData, size_t p_szSize, TypeFlags p_tfType)
	{

		if (p_szSize >= m_szBufferSize || p_szSize + m_szOccupiedSize[GetWriteSlot()] >= m_szBufferSize) {
			m_bSlotResizeState = true;
			ResizeBuffer(p_szSize);
		}


		int slot = GetWriteSlot();

		std::byte* l_pWriteLocation = static_cast<std::byte*>(m_pSlots[slot]) + m_szWriteCursor[slot];
		uint32_t count = 1;

		switch (p_tfType) {
		case TypeFlags::BUFFER_INSTANCE_DATA:
			count = p_szSize / sizeof(InstanceData);
			std::memcpy(reinterpret_cast<InstanceData*>(l_pWriteLocation), p_pData, p_szSize);
			break;
		case TypeFlags::BUFFER_CAMERA_DATA:
		case TypeFlags::BUFFER_ANIMATION_DATA:
		case TypeFlags::BUFFER_STATIC_MATRIX_DATA:
			count = p_szSize / sizeof(glm::mat4);
			std::memcpy(reinterpret_cast<glm::mat4*>(l_pWriteLocation), p_pData, p_szSize);
			break;
		case TypeFlags::BUFFER_DRAW_CALL_DATA:
			count = p_szSize / sizeof(DrawElementsIndirectCommand);
			std::memcpy(reinterpret_cast<DrawElementsIndirectCommand*>(l_pWriteLocation), p_pData, p_szSize);
			break;
		case TypeFlags::BUFFER_TEXTURE_DATA:
			count = p_szSize / sizeof(GLuint64);
			std::memcpy(reinterpret_cast<GLuint64*>(l_pWriteLocation), p_pData, p_szSize);
			break;
		case TypeFlags::BUFFER_LIGHT_DATA:
		case TypeFlags::BUFFER_PARTICLE_DATA:
		default:
			std::memcpy(l_pWriteLocation, p_pData, p_szSize);
			break;
		}


		uint32_t allocID = AllocateID(p_szSize);
		SAllocation& l_allocation = m_Allocations[allocID];

		l_allocation.alive = true;
		l_allocation.generation++;
		l_allocation.offset = m_szWriteCursor[slot];
		l_allocation.size = p_szSize;


		SBufferHandle handle;
		handle.allocationID = allocID;
		handle.bufferID = m_uiDynamicBufferID;
		handle.slot = GetDynamicSlotType(slot);
		handle.generation = l_allocation.generation;


		SBufferRange l_Range;
		l_Range.count = count;
		l_Range.dataType = p_tfType;
		l_Range.handle = handle;

		m_szWriteCursor[slot] += p_szSize;
		m_szOccupiedSize[slot] += p_szSize;
		


		return l_Range;





		return SBufferRange();
	}
	
	void CDynamicBuffer::UpdateRange(SBufferRange* p_brRange, const void* p_pData, size_t p_szDataSize)
	{
		*p_brRange = InsertNewData(p_pData, p_szDataSize, p_brRange->dataType);
	}

	void CDynamicBuffer::ResizeBuffer(size_t p_szMinimumSize)
	{
		//fuck if i know why it now accepts std::max ????
#ifdef PLATFORM_WINDOWS
		size_t l_newSize = std::max(2 * m_szBufferSize, p_szMinimumSize);
#elif defined(PLATFORM_LINUX)
		size_t l_newSize = std::max(2 * slotFullSize[i], minimumSize);
#endif


		if (m_bSlotResizeState) {
			if (m_bUseTrippleBuffering) {
				
				for (int i = 0; i < 3; i++) {

					WaitForSlotFence(i);

					GLuint l_gluiNewBuffer;
					glCreateBuffers(1, &l_gluiNewBuffer);
					glNamedBufferStorage(l_gluiNewBuffer, l_newSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
						GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

					glCopyNamedBufferSubData(m_uiSlotIDs[i], l_gluiNewBuffer, 0, 0, m_szOccupiedSize[i]);

					GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
					glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX);
					glDeleteSync(fence);

					glDeleteBuffers(1, &m_uiSlotIDs[i]);

					m_uiSlotIDs[i] = l_gluiNewBuffer;



				}

				MapAllBufferSlots();
				m_bSlotResizeState = false;
				return;
			}

			int l_slot = GetWriteSlot();
			WaitForSlotFence(l_slot);

			GLuint l_gluiNewBuffer;
			glCreateBuffers(1, &l_gluiNewBuffer);
			glNamedBufferStorage(l_gluiNewBuffer, l_newSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
				GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

			glCopyNamedBufferSubData(m_uiSlotIDs[l_slot], l_gluiNewBuffer, 0, 0, m_szOccupiedSize[l_slot]);

			GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX);
			glDeleteSync(fence);

			glDeleteBuffers(1, &m_uiSlotIDs[l_slot]);

			m_uiSlotIDs[l_slot] = l_gluiNewBuffer;

			m_pSlots[l_slot] = glMapNamedBuffer(l_gluiNewBuffer, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
				GL_MAP_COHERENT_BIT);

			m_bSlotResizeState = false;
			return;

		}



	}

	void CDynamicBuffer::ClearBuffer()
	{
		int slot = m_iNextSlot;
		m_szWriteCursor[slot] = 0;
		m_szOccupiedSize[slot] = 0;
		m_Allocations.clear();

		if (m_bSlotResizeState) {
			ResizeBuffer();
		}
	}

	std::optional<SAllocation> CDynamicBuffer::GetAllocation(int p_AllocationID)
	{

		if (p_AllocationID >= m_Allocations.size()) { return std::nullopt; }

		return m_Allocations[p_AllocationID];

		return std::nullopt;
	}

	void CDynamicBuffer::SetBinding(int p_iBinding)
	{
		m_iBinding = p_iBinding;
	}

	int CDynamicBuffer::GetBufferID()
	{
		return m_uiDynamicBufferID;
	}

	void CDynamicBuffer::BeginWritting()
	{
		
		if (m_bUseTrippleBuffering) {
			bool found = false;
			for (int i = 0; i < 3; ++i) {
				int candidate = (m_iCurrentSlot + i + 1) % 3;
				if (WaitForSlotFence(candidate)) {
					m_iNextSlot = candidate;
					found = true;
					break;
				}
			}

			if (!found) {
				// No free slot found, force wait on the oldest slot
				auto max = std::max_element(m_uiSlotAge, m_uiSlotAge + 3);
				int oldest = std::distance(m_uiSlotAge, max);

				GLenum waitRes = glClientWaitSync(
					m_glsFences[oldest], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
				if (waitRes == GL_TIMEOUT_EXPIRED)
					SDL_Log(
						"Warning: GPU still not finished with buffer slot %d, forced wait",
						oldest);

				glDeleteSync(m_glsFences[oldest]);
				m_glsFences[oldest] = 0;
				m_uiSlotAge[oldest] = 0;

				m_iNextSlot = oldest;
			}
		}
		else {
			m_iNextSlot = 0;
			m_iCurrentSlot = 0;
		}
		ClearBuffer();
	}

	void CDynamicBuffer::EndWritting()
	{
		glBindBuffer(m_gleTarget, m_uiSlotIDs[m_iNextSlot]);

		// Place fence for the slot we just wrote (nextSlot)
		// We set fence for nextSlot explicitly so waitForSlotFence checks the right
		// sync object.
		SetDownFence(m_iNextSlot);

		// Commit swap: nextSlot becomes current slot used for rendering
		m_iCurrentSlot = m_iNextSlot;
	}

	void CDynamicBuffer::Destroy()
	{
		for (int i = 0; i < 3; ++i) {
			if (m_pSlots[i]) {
				// unmap if mapped
				glUnmapNamedBuffer(m_uiSlotIDs[i]);
				m_pSlots[i] = nullptr;
			}
			if (m_glsFences[i]) {
				glDeleteSync(m_glsFences[i]);
				m_glsFences[i] = 0;
			}
			if (glIsBuffer(m_uiSlotIDs[i])) {
				glDeleteBuffers(1, &m_uiSlotIDs[i]);
			}
			m_uiSlotIDs[i] = 0;
		}
	}

	uint32_t CDynamicBuffer::AllocateID(size_t p_szMinimumSize)
	{

		//for now nothing since we arent storing Allocations for a free list


		m_Allocations.push_back(SAllocation());
		return m_Allocations.size() - 1;





		//return 0;
	}

	SlotType CDynamicBuffer::GetDynamicSlotType(int p_ID)
	{
		switch (p_ID) {
		case 0:
			return SlotType::DYNAMIC_SLOT_1;
			break;
		case 1:
			return SlotType::DYNAMIC_SLOT_2;
			break;
		case 2:
			return SlotType::DYNAMIC_SLOT_3;
			break;
		default:
			return SlotType::SLOT_NONE;
			break;
		}
	}

	int CDynamicBuffer::GetDynamicSlotID(SlotType p_type)
	{
		switch (p_type) {

		case SlotType::DYNAMIC_SLOT_1:
			return 0;
			break;

		case SlotType::DYNAMIC_SLOT_2:
			return 1;
			break;
		case SlotType::DYNAMIC_SLOT_3:
			return 2;
			break;
		case SlotType::SLOT_NONE:
			return -1;
		}
	}

	void CDynamicBuffer::SetDownFence(int p_Slot)
	{
		if (p_Slot < 0 || p_Slot >= 3)
			return;

		if (m_glsFences[p_Slot]) {
			glDeleteSync(m_glsFences[p_Slot]);
		}
		m_glsFences[p_Slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		// update age for the slot we just fenced
		m_uiSlotAge[p_Slot] = ++m_uiSlotTimeLine;
	}

	void CDynamicBuffer::MapAllBufferSlots()
	{
		for (unsigned int i = 0; i < 3; i++) {

			if (glIsBuffer(m_uiSlotIDs[i])) {

				m_pSlots[i] = glMapNamedBufferRange(
					m_uiSlotIDs[i], 0, m_szBufferSize,
					GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
			}
			
		}
	}

	bool CDynamicBuffer::WaitForSlotFence(int p_Slot)
	{
		if (m_glsFences[p_Slot] == 0)
			return true;

		// Flush commands to GPU before waiting
		GLenum res = glClientWaitSync(m_glsFences[p_Slot], GL_SYNC_FLUSH_COMMANDS_BIT, 0);

		if (res == GL_ALREADY_SIGNALED || res == GL_CONDITION_SATISFIED) {
			glDeleteSync(m_glsFences[p_Slot]);
			m_glsFences[p_Slot] = 0;
			return true;
		}

		return false;
	}

	void CDynamicBuffer::SetSlot(int p_Slot)
	{
		if (m_bUseTrippleBuffering) {
			m_iCurrentSlot = p_Slot;
		}
		else {
			m_iCurrentSlot = 0;
		}

		switch (m_gleTarget) {
		case GL_SHADER_STORAGE_BUFFER:
		case GL_UNIFORM_BUFFER:
		case GL_ATOMIC_COUNTER_BUFFER:
		case GL_TRANSFORM_FEEDBACK_BUFFER:
			// Indexed binding targets

			glBindBufferBase(m_gleTarget, m_iBinding, m_uiSlotIDs[m_iCurrentSlot]);
			break;
		case GL_DRAW_INDIRECT_BUFFER:
		case GL_ARRAY_BUFFER:
		case GL_ELEMENT_ARRAY_BUFFER:
			// Non-indexed binding targets
			glBindBuffer(m_gleTarget, m_uiSlotIDs[m_iCurrentSlot]);
			break;

		default:
			SDL_Log("SetSlot(): Unsupported Target=0x%X", m_gleTarget);
			break;
		}



	}

	uint32_t CDynamicBuffer::GetWriteSlot()
	{
		return m_iNextSlot;
	}

}
