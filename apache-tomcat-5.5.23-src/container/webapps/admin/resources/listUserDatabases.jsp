<!-- Standard Struts Entries -->
<%@ page language="java" import="java.net.URLEncoder" contentType="text/html;charset=utf-8" %>
<%@ taglib uri="http://struts.apache.org/tags-bean" prefix="bean" %>
<%@ taglib uri="http://struts.apache.org/tags-html" prefix="html" %>
<%@ taglib uri="http://struts.apache.org/tags-logic" prefix="logic" %>
<%@ taglib uri="/WEB-INF/controls.tld" prefix="controls" %>

<html:html locale="true">

<%@ include file="../users/header.jsp" %>

<!-- Body -->
<body bgcolor="white" background="../images/PaperTexture.gif">

<!--Form -->

<html:errors/>

<html:form action="/resources/listUserDatabases">

  <bean:define id="domainInfo" type="java.lang.String"
               name="userDatabasesForm" property="domain"/>
  <html:hidden property="domain"/>

  <table width="100%" border="0" cellspacing="0" cellpadding="0">
    <tr bgcolor="7171A5">
      <td width="81%">
        <div class="page-title-text" align="left">
          <bean:message key="resources.treeBuilder.databases"/>
        </div>
      </td>
      <td width="19%">
        <div align="right">
          <%@ include file="listUserDatabases.jspf" %>
        </div>
      </td>
    </tr>
  </table>

<br>

<%@ include file="userDatabases.jspf" %>

</html:form>

<br>

</body>
</html:html>
