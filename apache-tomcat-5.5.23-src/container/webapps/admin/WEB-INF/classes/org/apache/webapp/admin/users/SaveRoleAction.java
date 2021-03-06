/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


package org.apache.webapp.admin.users;


import java.io.IOException;
import java.net.URLDecoder;
import java.util.Locale;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import javax.management.Attribute;
import javax.management.MBeanServer;
import javax.management.ObjectName;
import org.apache.struts.util.MessageResources;
import org.apache.webapp.admin.ApplicationServlet;
import org.apache.webapp.admin.TomcatTreeBuilder;

/**
 * <p>Implementation of <strong>Action</strong> that saves a new or
 * updated Role back to the underlying database.</p>
 *
 * @author Craig R. McClanahan
 * @version $Revision: 479041 $ $Date: 2006-11-24 16:02:20 -0700 (Fri, 24 Nov 2006) $
 * @since 4.1
 */

public final class SaveRoleAction extends Action {


    // ----------------------------------------------------- Instance Variables


    /**
     * The MBeanServer we will be interacting with.
     */
    private MBeanServer mserver = null;


    // --------------------------------------------------------- Public Methods


    /**
     * Process the specified HTTP request, and create the corresponding HTTP
     * response (or forward to another web component that will create it).
     * Return an <code>ActionForward</code> instance describing where and how
     * control should be forwarded, or <code>null</code> if the response has
     * already been completed.
     *
     * @param mapping The ActionMapping used to select this instance
     * @param actionForm The optional ActionForm bean for this request (if any)
     * @param request The HTTP request we are processing
     * @param response The HTTP response we are creating
     *
     * @exception IOException if an input/output error occurs
     * @exception ServletException if a servlet exception occurs
     */
    public ActionForward execute(ActionMapping mapping,
                                 ActionForm form,
                                 HttpServletRequest request,
                                 HttpServletResponse response)
        throws IOException, ServletException {

        // Look up the components we will be using as needed
        if (mserver == null) {
            mserver = ((ApplicationServlet) getServlet()).getServer();
        }
        MessageResources resources = getResources(request);
        Locale locale = getLocale(request);

        // Has this transaction been cancelled?
        if (isCancelled(request)) {
            return (mapping.findForward("List Roles Setup"));
        }

        // Check the transaction token
        if (!isTokenValid(request)) {
            response.sendError
                (HttpServletResponse.SC_BAD_REQUEST,
                 resources.getMessage(locale, "users.error.token"));
            return (null);
        }

        // Perform any extra validation that is required
        RoleForm roleForm = (RoleForm) form;
        String databaseName =
            URLDecoder.decode(roleForm.getDatabaseName(),TomcatTreeBuilder.URL_ENCODING);
        String objectName = roleForm.getObjectName();

        // Perform an "Add Role" transaction
        if (objectName == null) {

            String signature[] = new String[2];
            signature[0] = "java.lang.String";
            signature[1] = "java.lang.String";

            Object params[] = new Object[2];
            params[0] = roleForm.getRolename();
            params[1] = roleForm.getDescription();

            ObjectName oname = null;

            try {

                // Construct the MBean Name for our UserDatabase
                oname = new ObjectName(databaseName);

                // Create the new object and associated MBean
                mserver.invoke(oname, "createRole",
                               params, signature);

            } catch (Exception e) {

                getServlet().log
                    (resources.getMessage(locale, "users.error.invoke",
                                          "createRole"), e);
                response.sendError
                    (HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                     resources.getMessage(locale, "users.error.invoke",
                                          "createRole"));
                return (null);
            }

        }

        // Perform an "Update Role" transaction
        else {

            ObjectName oname = null;
            String attribute = null;

            try {

                // Construct an object name for this object
                oname = new ObjectName(objectName);

                // Update the specified role
                attribute = "description";
                mserver.setAttribute
                    (oname,
                     new Attribute(attribute, roleForm.getDescription()));

            } catch (Exception e) {

                getServlet().log
                    (resources.getMessage(locale, "users.error.set.attribute",
                                          attribute), e);
                response.sendError
                    (HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                     resources.getMessage(locale, "users.error.set.attribute",
                                          attribute));
                return (null);

            }

        }

        // Save the updated database information
        try {

            ObjectName dname = new ObjectName(databaseName);
            mserver.invoke(dname, "save",
                           new Object[0], new String[0]);

        } catch (Exception e) {

            getServlet().log
                (resources.getMessage(locale, "users.error.invoke",
                                      "save"), e);
            response.sendError
                (HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                 resources.getMessage(locale, "users.error.invoke",
                                      "save"));
            return (null);

        }

        // Proceed to the list roles screen
        return (mapping.findForward("Roles List Setup"));

    }


}
