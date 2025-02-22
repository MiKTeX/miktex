#pragma once
#include <string>
#include <optional>
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/lsp/general/InitializeParams.h"

struct LintRule
{
    std::string key;
    std::string name;

    std::string Display() const
    {
        return name + " (" + key + ")";
    }
    bool activeByDefault = true;
    optional<std::string> severity;
    optional<std::string> type;
    int icon_index = -1;
    MAKE_SWAP_METHOD(LintRule, key, name, activeByDefault, severity, type);
};
MAKE_REFLECT_STRUCT(LintRule, key, name, activeByDefault, severity, type);

struct RuleParameter
{
    std::string name;
    optional<std::string> description;
    optional<std::string> defaultValue;
};
MAKE_REFLECT_STRUCT(RuleParameter, name, description, defaultValue);

struct ShowRuleDescriptionParams
{

    optional<std::string> key;

    optional<std::string> name;

    optional<std::string> htmlDescription;

    optional<std::string> type;

    optional<std::string> severity;

    optional<std::vector<RuleParameter>> parameters;
    MAKE_SWAP_METHOD(ShowRuleDescriptionParams, key, name, htmlDescription, type, severity, parameters)
};
MAKE_REFLECT_STRUCT(ShowRuleDescriptionParams, key, name, htmlDescription, type, severity, parameters);

struct GetJavaConfigResponse
{
    std::string projectRoot;
    std::string sourceLevel;
    std::vector<std::string> classpath;
    bool isTest;
    std::string vmLocation;
    MAKE_SWAP_METHOD(GetJavaConfigResponse, projectRoot, sourceLevel, classpath, isTest, vmLocation);
};
MAKE_REFLECT_STRUCT(GetJavaConfigResponse, projectRoot, sourceLevel, classpath, isTest, vmLocation);

struct SetTraceNotificationParams
{
    lsInitializeParams::lsTrace value;
};
MAKE_REFLECT_STRUCT(SetTraceNotificationParams, value);

struct ServerConnectionSettings
{

    std::string SONARCLOUD_URL = "https://sonarcloud.io";
    std::vector<std::string> SONARCLOUD_ALIAS = {
        "https://sonarqube.com", "https://www.sonarqube.com", "https://www.sonarcloud.io", "https://sonarcloud.io"
    };

    std::string connectionId;
    std::string serverUrl;
    std::string token;
    optional<std::string> organizationKey;
    MAKE_SWAP_METHOD(ServerConnectionSettings, connectionId, serverUrl, token, organizationKey)
};
MAKE_REFLECT_STRUCT(ServerConnectionSettings, connectionId, serverUrl, token, organizationKey)

struct RuleSetting
{
    bool IsOn();
    std::string level = "on";
    RuleSetting(bool activate);
    RuleSetting() = default;
    void toggle();
    void on()
    {
        level = "on";
    }
    void off()
    {
        level = "off";
    }
    void turn(bool activate)
    {
        if (activate)
        {
            on();
        }
        else
        {
            off();
        }
    }
    optional<std::map<std::string, std::string>> parameters;
};
MAKE_REFLECT_STRUCT(RuleSetting, level, parameters)

struct ConsoleParams
{
    optional<bool> showAnalyzerLogs;
    optional<bool> showVerboseLogs;
    MAKE_SWAP_METHOD(ConsoleParams, showAnalyzerLogs, showVerboseLogs)
};
MAKE_REFLECT_STRUCT(ConsoleParams, showAnalyzerLogs, showVerboseLogs)

struct SonarLintWorkspaceSettings
{
    optional<bool> disableTelemetry;
    optional<std::map<std::string, ServerConnectionSettings>> connectedMode;
    optional<std::map<std::string, RuleSetting>> rules;
    optional<ConsoleParams> output;

    optional<std::string> pathToNodeExecutable;

    optional<std::map<std::string, std::string>> getConfigurationParameters(std::string const& ruleKey);
};

MAKE_REFLECT_STRUCT(SonarLintWorkspaceSettings, disableTelemetry, connectedMode, rules, output, pathToNodeExecutable)

DEFINE_REQUEST_RESPONSE_TYPE(slls_listAllRules, JsonNull, lsp::Any, "sonarlint/listAllRules");

DEFINE_NOTIFICATION_TYPE(Notify_didClasspathUpdate, lsDocumentUri, "sonarlint/didClasspathUpdate")

DEFINE_NOTIFICATION_TYPE(Notify_didJavaServerModeChange, std::string, "sonarlint/didJavaServerModeChange")

DEFINE_REQUEST_RESPONSE_TYPE(slls_showSonarLintOutput, JsonNull, JsonNull, "sonarlint/showSonarLintOutput");

DEFINE_REQUEST_RESPONSE_TYPE(slls_openJavaHomeSettings, JsonNull, JsonNull, "sonarlint/openJavaHomeSettings");

DEFINE_REQUEST_RESPONSE_TYPE(slls_openPathToNodeSettings, JsonNull, JsonNull, "sonarlint/openPathToNodeSettings");

DEFINE_REQUEST_RESPONSE_TYPE(
    slls_showRuleDescription, ShowRuleDescriptionParams, JsonNull, "sonarlint/showRuleDescription"
);

DEFINE_REQUEST_RESPONSE_TYPE(slls_getJavaConfig, lsDocumentUri, GetJavaConfigResponse, "sonarlint/getJavaConfig");

DEFINE_NOTIFICATION_TYPE(slls_setTraceNotification, SetTraceNotificationParams, "$/setTraceNotification")
