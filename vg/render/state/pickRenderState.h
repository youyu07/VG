#pragma once


namespace vg
{
	class PickRenderState
	{
		VkFormat colorFormat = VK_FORMAT_R32G32_UINT;

		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
		vk::RenderPass renderPass;

		vk::Image color;
		vk::Image depth;
		vk::Image copy;
		vk::FrameBuffer frameBuffer;

		vk::CommandBuffer cmd;

		VkExtent2D curExtent = {};
	public:
		PickRenderState() {}

		PickRenderState(const Context& ctx, const vk::DescriptorSetLayout& cameraSetLayout) {
			setupRenderPass(ctx);
			vk::PipelineLayoutMaker plm;
			plm.setLayout(cameraSetLayout);
			plm.pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t));
			layout = plm.create(ctx->getDevice());
			setupPipeline(ctx);

			cmd = ctx->getCommandPool()->createCommandBuffer();
		}

		SelectInfo select(const Context& ctx,const vk::DescriptorSet& cameraSet,GeometryManager& geometries, const glm::uvec2& point) {
			auto extent = VkExtent2D{ ctx->getExtent().width,ctx->getExtent().height };
			//extent = { extent.width - extent.width % 2, extent.height - extent.height % 2 };
			if ((curExtent.width != extent.width) && (curExtent.height != extent.height)) {
				resize(ctx, extent);

				copy = ctx->getDevice()->createTransferImage(extent.width, extent.height, colorFormat);
			}

			cmd->begin();

			VkRect2D area = { {},extent };

			std::array<VkClearValue, 2> clearValue = { VkClearColorValue{},{1.0f,0} };
			cmd->beginRenderPass(renderPass, frameBuffer, area, clearValue);

			cmd->viewport(0, 0, extent.width, extent.height);
			cmd->scissor(0, 0, extent.width, extent.height);

			cmd->bindPipeline(pipeline);
			uint32_t setOffset = { 0 };
			cmd->bindDescriptorSet(layout, 0, cameraSet->get(), setOffset);

			geometries.draw([&](uint32_t id,const GeometryBuffer& g) {
				uint32_t pc[1] = { id+1 };

				cmd->pushContants(layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, pc);

				VkDeviceSize offset = { 0 };
				cmd->bindVertexBuffer(0, g.vertexBuffer->get(), offset);
				cmd->bindIndexBuffer(g.indexBuffer->get(), 0, g.indexType);
				cmd->drawIndexd(g.count, 1);
			});

			cmd->endRenderPass();

			copy->setLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			color->setLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			VkImageCopy imageCopy = { color->getSubresourceLayers(0),{},copy->getSubresourceLayers(0),{},{extent.width,extent.height,1} };
			cmd->copyImage(color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, copy, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageCopy);

			copy->setLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
			color->setLayout(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			cmd->end();

			ctx->getGraphicsQueue()->submit(cmd);
			ctx->getGraphicsQueue()->waitIdle();

			auto ptr = (SelectInfo*)copy->map();
			auto sel = glm::uvec2(point.x, point.y);
			auto d = ptr[sel.y * extent.width + sel.x];
			copy->unmap();

			//log_info(d.ObjectID, "    ", d.PrimID);

			return d;
		}

	private:
		void setupRenderPass(const Context& ctx)
		{
			vk::RenderpassMaker rm;
			rm.attachmentBegin(colorFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.attachmentBegin(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.subpassBegin();
			rm.subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0);
			rm.subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
			renderPass = rm.create(ctx->getDevice());
		}

		void setupPipeline(const Context& ctx) {
			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(location=1)in vec3 normal;\n"
				"layout(location=2)in vec2 texcoord;\n"
				"layout(set=0,binding=0) uniform CameraMatrix {\n"
				"	mat4 projection;\n"
				"	mat4 view;\n"
				"} matrix;\n"

				"void main(){\n"
				"	gl_Position = matrix.projection * matrix.view * vec4(position,1.0);\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(push_constant) uniform PushConstant {\n"
				"	uint objectIndex;\n"
				"} pc;\n"

				"layout(location=0)out uvec2 color;\n"
				"void main(){\n"
				"	color = uvec2(pc.objectIndex, gl_PrimitiveID + 1);\n"
				"}";

			auto pm = vk::PipelineMaker(ctx->getDevice()).defaultBlend(VK_FALSE).defaultDynamic();
			pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
			pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
			pm.vertexBinding(0, 32);
			pm.vertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
			pm.vertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12);
			pm.vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, 24);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pipeline = pm.create(layout, renderPass);
		}

		void resize(const Context& ctx, const VkExtent2D& extent) {
			color = ctx->getDevice()->createColorAttachment(extent.width, extent.height, VK_SAMPLE_COUNT_1_BIT, colorFormat);
			depth = ctx->getDevice()->createDepthStencilAttachment(extent.width, extent.height);

			std::array<VkImageView, 2> attachments = { color->view(),depth->view() };
			frameBuffer = ctx->getDevice()->createFrameBuffer(renderPass, extent.width, extent.height, attachments);

			curExtent = extent;
		}
	};

}