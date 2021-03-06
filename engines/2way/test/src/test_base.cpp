/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef TEST_BASE_H_INCLUDED
#include "test_base.h"
#endif

#ifndef PV_LOGGER_IMPL_H_INCLUDED
#include "pv_logger_impl.h"
#endif


#include "pv_mime_string_utils.h"

#if defined(__linux__) || defined(linux)
#define CONFIG_FILE_PATH _STRLIT("")
#endif

#include "pv_2way_codecspecifier.h"

uint32 test_base::iTestCounter = 0;

void test_base::H324MConfigCommandCompletedL(PVMFCmdResp& aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}

void test_base::H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification)
{
    OSCL_UNUSED_ARG(aNotification);
}

void test_base::CreateH324Component(bool aCreateH324)
{
    OSCL_UNUSED_ARG(aCreateH324);

    /*!

      Step 4b: Create Component
      First step in creating h324m ocmponent- querying for the TSC node using the PVH324MConfigUuid UUID.
    */
    // get TSC node
    iQueryInterfaceCmdId = terminal->QueryInterface(PVH324MConfigUuid, iTempH324MConfigIterface);

}

void test_base::QueryInterfaceSucceeded()
{
    /*!

      Step 6b: Query H324m component
      Using the appropriate uuid, and the TSC node, query for the H324m component
    */
    if (iH324MConfig)
    {
        iH324MConfig->SetObserver(this);

        PVUuid uuid = PVUuidH324ComponentInterface;

        iH324MConfig->queryInterface(uuid, (PVInterface*&)iComponentInterface);
    }
    // now we have created the component, we can do init.
    Init();
}

bool test_base::Init()
{
    int32 error = 0;
    OSCL_FastString aStr;

    /*!

      Step 7: Initialize terminal
      @param PV2Way324InitInfo
      Initialize the terminal using PV2Way324InitInfo structure to set
      the formats that the application supports
    */
    OSCL_TRY(error, iInitCmdId = terminal->Init(iSdkInitInfo));
    if (error)
    {
        printf("\n*************** Test FAILED: could not initialize terminal *************** \n");
        test_is_true(false);

        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }

        terminal = NULL;
        return false;
    }
    return true;
}


void test_base::InitSucceeded()
{
    connect();
}

void test_base::InitFailed()
{
    printf("\n*************** Test FAILED: InitFailed *************** \n");
    if (timer)
    {
        timer->Cancel();
    }
    RunIfNotReady();
}
void test_base::InitCancelled()
{
    InitFailed();
}
void test_base::EncoderIFSucceeded()
{
}

void test_base::EncoderIFFailed()
{
    printf("\n*************** Test FAILED: EncoderIF Failed *************** \n");
}

void test_base::ConnectSucceeded()
{
    disconnect();
}

void test_base::ConnectFailed()
{
    printf("\n*************** Test FAILED: connect failed *************** \n");
    test_is_true(false);
    reset();
}

void test_base::ConnectCancelled()
{
    ConnectFailed();
}

void test_base::CancelCmdCompleted()
{
    test_is_true(true);
    RunIfNotReady();
}

void test_base::RstCmdCompleted()
{
    cleanup();
    RunIfNotReady();
}

void test_base::DisCmdSucceeded()
{
    reset();
}

void test_base::DisCmdFailed()
{
    reset();
    printf("test_base::DisCmdFailed() \n");
}

void test_base::AudioAddSinkSucceeded()
{
    iAudioSinkAdded = true;
    if (iAudioSourceAdded && timer)
        timer->RunIfNotReady(TEST_DURATION);
}

void test_base::AudioAddSourceSucceeded()
{
    iAudioSourceAdded = true;
    if (iAudioSinkAdded && timer)
        timer->RunIfNotReady(TEST_DURATION);
}

void test_base::AudioAddSinkFailed()
{
    printf("\n****** Test FAILED: add audio sink failed \n");
    test_is_true(false);
    disconnect();
}

void test_base::AudioAddSourceFailed()
{
    printf("\n****** Test FAILED: add audio source failed \n");
    test_is_true(false);
    disconnect();
}

void test_base::VideoAddSinkSucceeded()
{
    iVideoSinkAdded = true;
}

void test_base::VideoAddSinkFailed()
{
    printf("\n***** Test FAILED: add video sink failed  \n");
    test_is_true(false);
    disconnect();
}

void test_base::VideoAddSourceSucceeded()
{
    iVideoSourceAdded = true;
}

void test_base::VideoAddSourceFailed()
{
    printf("\n***** Test FAILED: add video source failed \n");
    iVideoSourceAdded = false;
}

void test_base::AudioRemoveSourceCompleted()
{
    iAudioSourceAdded = false;
    if (!iAudioSinkAdded)
        disconnect();
}

void test_base::AudioRemoveSinkCompleted()
{
    iAudioSinkAdded = false;
    if (!iAudioSourceAdded)
        disconnect();
}


void test_base::VideoRemoveSourceCompleted()
{
    int error = 0;
    iVideoSourceAdded = false;
    OSCL_TRY(error, iVideoRemoveSinkId = iSourceAndSinks->RemoveVideoSink());
    if (error)
    {
        printf("\n*************** Test FAILED: error removing video sink *************** \n");
        test_is_true(false);
        disconnect();
    }
}
void test_base::VideoRemoveSinkCompleted()
{
    iVideoSinkAdded = false;
    if (!iVideoSourceAdded)
        disconnect();
}


void test_base::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVCommandId cmdId = aResponse.GetCmdId();
    if (cmdId < 0)
        return;
    iTestStatus &= (aResponse.GetCmdStatus() == PVMFSuccess) ? true : false;

    if (iQueryInterfaceCmdId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            if (iTempH324MConfigIterface)
            {
                /*!

                  Step 6: Finish Querying Component

                  Step 6a: Receive notification that TSC node interface pointer has been set.
                  At this point we are notified that the TSC node interface pointer is valid.
                  Convert the pointer to H324MConfigInterface
                */
                iH324MConfig = OSCL_STATIC_CAST(H324MConfigInterface*, iTempH324MConfigIterface);
                iH324MConfig->addRef();
                iTempH324MConfigIterface->removeRef();
                iTempH324MConfigIterface = NULL;
                QueryInterfaceSucceeded();
            }

        }
    }
    else if (iInitCmdId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            InitSucceeded();
        }
        else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
        {
            InitCancelled();
        }
        else
        {
            InitFailed();
        }
    }
    else if (iEncoderIFCommandId == cmdId)
    {
        if (aResponse.GetCmdStatus() != PVMFSuccess)
        {
            EncoderIFFailed();
        }
        else
        {
            EncoderIFSucceeded();
        }
    }
    else if (iConnectCmdId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            ConnectSucceeded();
        }
        else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
        {
            ConnectCancelled();
        }
        else
        {
            ConnectFailed();
        }
    }
    else if (iDisCmdId == cmdId)
    {
        if (aResponse.GetCmdStatus() != PVMFSuccess)
        {
            DisCmdFailed();
        }
        else
        {
            DisCmdSucceeded();
        }
    }
    else if (iRstCmdId == cmdId)
    {
        RstCmdCompleted();
    }
    else if (iAudioAddSinkId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            AudioAddSinkSucceeded();
        }
        else
        {
            AudioAddSinkFailed();
        }
    }
    else if (iAudioAddSourceId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            AudioAddSourceSucceeded();
        }
        else
        {
            AudioAddSourceFailed();
        }
    }
    else if (iAudioRemoveSourceId == cmdId)
    {
        AudioRemoveSourceCompleted();
    }
    else if (iAudioRemoveSinkId == cmdId)
    {
        AudioRemoveSinkCompleted();
    }
    else if (iVideoAddSinkId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            VideoAddSinkSucceeded();
        }
        else
        {
            VideoAddSinkFailed();
        }
    }
    else if (iVideoAddSourceId == cmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            VideoAddSourceSucceeded();
        }
        else
        {
            VideoAddSourceFailed();
        }
    }
    else if (iVideoRemoveSourceId == cmdId)
    {
        VideoRemoveSourceCompleted();
    }
    else if (iVideoRemoveSinkId == cmdId)
    {
        VideoRemoveSinkCompleted();
    }
    else if (iCancelCmdId == cmdId)
    {
        CancelCmdCompleted();
    }
}


void test_base::InitializeLogs()
{
    uint32 error = 0;
    PVLoggerConfigFile obj;
    obj.SetConfigFilePath(CONFIG_FILE_PATH);
    error = 0;
    if (obj.IsLoggerConfigFilePresent())
    {
        error = obj.SetLoggerSettings(terminal, TEST_LOG_FILENAME);
        if (0 != error)
        {
            printf("Error Occured in PVLoggerConfigFile::SetLoggerSettings() \n");
        }
        else
        {
            //sucess able to set logger settings
            return;
        }
    }

    PVLoggerAppender *lLoggerAppender = 0;
    OsclRefCounter *refCounter = NULL;
    bool logfile = true;
    if (logfile)
    {
        //File Log
        const uint32 TEXT_FILE_APPENDER_CACHE_SIZE = 1024;
        lLoggerAppender = TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender(TEST_LOG_FILENAME,
                          TEXT_FILE_APPENDER_CACHE_SIZE);
        OsclRefCounter *appenderRefCounter = new OsclRefCounterSA<AppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(lLoggerAppender);
        refCounter = appenderRefCounter;
    }
    else
    {
        //Console Log
        lLoggerAppender = new StdErrAppender<TimeAndIdLayout, 1024>();
        OsclRefCounter *appenderRefCounter = new OsclRefCounterSA<AppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >(lLoggerAppender);
        refCounter = appenderRefCounter;
    }
    OsclSharedPtr<PVLoggerAppender> appenderPtr(lLoggerAppender, refCounter);
    terminal->SetLogLevel("", PVLOGMSG_DEBUG, true);
    terminal->SetLogAppender("", appenderPtr);

}

bool test_base::start_async_test()
{
    int error = 0;
    /*!

      Step 2: Create terminal
      Create proxy/non-proxy terminal using CPV2WayProxyFactory or CPV2WayEngineFactory
      @param PVCommandStatusObserver, PVInformationalEventObserver, PVErrorEventObserver to receive notifications from SDK
    */
    if (iUseProxy)
    {
        OSCL_TRY(error, terminal = CPV2WayProxyFactory::CreateTerminal(PV_324M,
                                   (PVCommandStatusObserver *) this,
                                   (PVInformationalEventObserver *) this,
                                   (PVErrorEventObserver *) this));
    }
    else
    {
        OSCL_TRY(error, terminal = CPV2WayEngineFactory::CreateTerminal(iSIP ? PV_SIP : PV_324M,
                                   (PVCommandStatusObserver *) this,
                                   (PVInformationalEventObserver *) this,
                                   (PVErrorEventObserver *) this));
    }

    if (error)
    {
        printf("\n*************** Test FAILED: error creating terminal *************** \n");
        test_is_true(false);
        return false;
    }

    if (iSourceAndSinks && terminal)
    {
        iSourceAndSinks->SetTerminal(terminal);
    }
    /*!

      Step 3: Initialize logs
      Initialize logs, send parameters to the terminal
    */
    InitializeLogs();

    iInitCmdId = -1;
    iQueryInterfaceCmdId = -1;
    CreateParts();
    return true;
}

void test_base::CreateParts()
{
    /*!

      Step 4: Create Communication, H324 component
      Create communication node, query for the H324 component
      In this case the communication node is a loopback node.
    */
    create_comm();
    CreateH324Component();
}

void test_base::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
            /*!

              Step 9: Receive track indications
              Receive either PVT_INDICATION_OUTGOING_TRACK or PVT_INDICATION_INCOMING_TRACK
              through the PVInformationalEventObserver interface
            */
        case PVT_INDICATION_OUTGOING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("\n\nIndication with logical channel #%d \n", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO)
            {
                if (iUsingAudio)
                {
                    iAudioAddSourceId = iSourceAndSinks->HandleOutgoingAudio(aEvent);
                    if (iAudioAddSourceId)
                    {
                        printf("Audio");
                    }
                    else
                    {
                        printf("\nError adding handling outgoing audio track\n");
                    }
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                if (iUsingVideo)
                {
                    iVideoAddSourceId = iSourceAndSinks->HandleOutgoingVideo(aEvent);
                    if (iVideoAddSourceId)
                    {
                        printf("Video");
                    }
                    else
                    {
                        printf("\nError adding handling outgoing video track\n");
                    }
                }
            }
            else
            {
                printf("unknown");
            }
            printf(" outgoing Track\n");
            break;
        }

        case PVT_INDICATION_INCOMING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("\n\nIndication with logical channel #%d \n", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO)
            {
                if (iUsingAudio)
                {
                    iAudioAddSinkId = iSourceAndSinks->HandleIncomingAudio(aEvent);
                    if (iAudioAddSinkId)
                    {
                        printf("Audio");
                    }
                    else
                    {
                        printf("\nError adding handling incoming audio track \n");
                    }
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                if (iUsingVideo)
                {
                    iVideoAddSinkId = iSourceAndSinks->HandleIncomingVideo(aEvent);
                    if (iVideoAddSinkId)
                    {
                        printf("Video");
                    }
                    else
                    {
                        printf("\nError adding handling incoming video track\n");
                    }
                }
            }
            else
            {
                printf("unknown");
            }
            printf(" incoming Track\n");
            break;
        }

        case PVT_INDICATION_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            break;

        case PVT_INDICATION_CLOSE_TRACK:
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

void test_base::TestCompleted()
{
    char name[128];
    oscl_snprintf(name, 128, "Test %.2d: %s", iTestNum, iTestName.get_cstr());
    m_last_result.set_name(name);

    // Print out the result for this test case
    fprintf(fileoutput, "\nResults for Test Case %d:\n", iTestNum);
    fprintf(fileoutput, "Successes %.2d, Failures %d\n"
            , m_last_result.success_count(), m_last_result.failures().size());
    fflush(fileoutput);
}
