/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Graphics/RenderGraph/RenderPass.h"
#include "API/FBO.h"
#include "Graphics/FullScreenPass.h"
#include "API/Sampler.h"
#include "Graphics/Program/ProgramVars.h"
#include <memory>

namespace Falcor
{
    class RenderContext;
    class Texture;
    class Fbo;
    class Gui;

    /** Seperable Screen-Space Subsurface Scattering Technique. Requires diffuse lit texture, depth texture, and mask for accurate output
    */
    class SubsurfaceScattering : public RenderPass, inherit_shared_from_this<RenderPass, SubsurfaceScattering>
    {
    public:
        using UniquePtr = std::unique_ptr<SubsurfaceScattering>;
        using SharedPtr = std::unique_ptr<SubsurfaceScattering>;
        /** Destructor
        */
        ~SubsurfaceScattering();

        /** Create a new object
        \param[in] kernelSize Number of samples taken along each axis
        \param[in] sigma Gaussian distribution sigma value used to calculate sample weights. Values smaller than twice the sigma are ineffective
        */
        static UniquePtr create(uint32_t kernelSize = 20, float scatteringWidth = 1.0f, const glm::vec3& color = {1.0f, 1.0f, 1.0f});

        static SubsurfaceScattering::UniquePtr deserialize(const RenderPassSerializer& serializer);
        
        RenderPassSerializer serialize();

        void execute(RenderContext* pRenderContext, Texture::SharedPtr pDiffuseSrc, Texture::SharedPtr pSrcDepth, Fbo::SharedPtr pDst, Texture::SharedPtr pSrcMaskTex);

        /** Called once before compilation. Describes I/O requirements of the pass.
        The requirements can't change after the graph is compiled. If the IO requests are dynamic, you'll need to trigger compilation of the render-graph yourself.
        */
        virtual void reflect(RenderPassReflection& reflector) const override;

        virtual void execute(RenderContext* pRenderContext, const RenderData* pData) override;

        /** Render UI controls for blur settings.
        \param[in] pGui GUI instance to render UI elements with
        \param[in] uiGroup Optional name. If specified, UI elements will be rendered within a named group
        */
        void renderUI(Gui* pGui, const char* uiGroup = nullptr);

        enum SubsurfaceScatteringMode
        {
            Translucent,
            Skin
        };

    private:
        SubsurfaceScattering(uint32_t kernelWidth, float scatteringWidth, const glm::vec3& color);
        
        void createTmpFbo(const Texture* pSrc);
        void createProgram();
        void updateDiffusionProfile();

        FullScreenPass::UniquePtr mpBlurPass;
        Fbo::SharedPtr mpTargetFbo;
        Fbo::SharedPtr mpTmpFbo;
        Sampler::SharedPtr mpSampler;
        bool mDirty = true;
        TypedBuffer<float>::SharedPtr mpWeights;
        GraphicsVars::SharedPtr mpVars;
        ParameterBlockReflection::BindLocation mSrcTexLoc;
        ParameterBlockReflection::BindLocation mSrcDepthTexLoc;
        ParameterBlockReflection::BindLocation mSrcOcclTexLoc;

        SubsurfaceScatteringMode mMode;
        uint32_t mKernelWidth;
        float mScatteringWidth;
        glm::vec3 mColor;
        float mStrength;
    };
}