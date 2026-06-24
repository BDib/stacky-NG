#include "NotificationManager.h"
#include "TrayIcon.h"
#include "Utils.h"
#include <windows.h>
#include <roapi.h>
#include <shellapi.h>
#include <windows.ui.notifications.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.data.xml.dom.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;

void NotificationManager::ShowNotification(const std::wstring& title, const std::wstring& message, HWND hWndFallback) {
    bool toastSent = false;

    HStringReference app_id(L"StackyManager");

    ComPtr<IToastNotificationManagerStatics> toastStatics;
    if (SUCCEEDED(RoGetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), __uuidof(IToastNotificationManagerStatics), &toastStatics))) {
        ComPtr<IXmlDocument> toastXml;
        if (SUCCEEDED(toastStatics->GetTemplateContent(ToastTemplateType_ToastText02, &toastXml))) {
            ComPtr<IXmlNodeList> nodeList;
            if (SUCCEEDED(toastXml->GetElementsByTagName(HStringReference(L"text").Get(), &nodeList))) {
                UINT32 length = 0;
                nodeList->get_Length(&length);
                if (length >= 2) {
                    auto setNodeText = [&](UINT32 index, const std::wstring& text) {
                        ComPtr<IXmlNode> node;
                        if (SUCCEEDED(nodeList->Item(index, &node))) {
                            ComPtr<IXmlText> textNode;
                            if (SUCCEEDED(toastXml->CreateTextNode(HStringReference(text.c_str()).Get(), &textNode))) {
                                ComPtr<IXmlNode> textAsNode;
                                if (SUCCEEDED(textNode.As(&textAsNode))) {
                                    ComPtr<IXmlNode> appended;
                                    node->AppendChild(textAsNode.Get(), &appended);
                                }
                            }
                        }
                    };
                    setNodeText(0, title);
                    setNodeText(1, message);

                    ComPtr<IToastNotificationFactory> toastFactory;
                    if (SUCCEEDED(RoGetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), __uuidof(IToastNotificationFactory), &toastFactory))) {
                        ComPtr<IToastNotification> toast;
                        if (SUCCEEDED(toastFactory->CreateToastNotification(toastXml.Get(), &toast))) {
                            ComPtr<IToastNotifier> notifier;
                            if (SUCCEEDED(toastStatics->CreateToastNotifierWithId(app_id.Get(), &notifier))) {
                                if (SUCCEEDED(notifier->Show(toast.Get()))) {
                                    toastSent = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!toastSent && hWndFallback) {
        NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
        nid.hWnd = hWndFallback;
        nid.uID = ID_TRAY_ICON;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        wcscpy_s(nid.szInfoTitle, title.c_str());
        wcscpy_s(nid.szInfo, message.c_str());
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
}
