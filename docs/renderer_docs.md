# Slate Engine vulkan rendering system

Architecture & engine API information

---

## Architectural Overview

Slate Engine's Vulkan rendering system is a high-performance, cross-platform, multi-pass, hardware-accelerated rendering architecture built on Vulkan 1.3 and SDL3. it was engineered for strict predictable behavior, minimal CPU latency, and direct-mapped buffer operations. the renderer abstracts complex, boring, Vulkan stuff behind a virtual interface (`slate::Renderer`) while still maintaining low-level hardware performance.

The renderer relies on strict multi-pass synchronization, mapped UBO and SSBO memory layouts for zero-copy CPU to GPU streaming, bindless descriptor indexing, and direct SDL3 integration.

### Core Architecture:

* Vulkan 1.3 spec
* Dual render passes
* Offscreen viewport copybuffer
* Mapped SSBO array

---

## Initialization and device context lifestyle:

the initialization sequence is as follows:

```
  void VulkanRenderer::init() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createDepthResources();
    createCommandPool();
    createSceneColorResources();
    createRenderPass();
    createTransparentRenderPass();
    createFramebuffers();
    createSyncObjects();
    createCommandBuffer();
    createDescriptorSetLayout();
    createUIDescriptorSetLayout();
    createUniformBuffers();
    createMaterialBuffers();
    createDescriptorPoolAndSets();
    createGraphicsPipeline();
    createGridPipeline();
    createGizmoPipeline();
    createUIGraphicsPipeline();
    createFontTexture();
    createUIDescriptorSet();
    createGridMesh();
    createGizmoMesh();
  }
```
