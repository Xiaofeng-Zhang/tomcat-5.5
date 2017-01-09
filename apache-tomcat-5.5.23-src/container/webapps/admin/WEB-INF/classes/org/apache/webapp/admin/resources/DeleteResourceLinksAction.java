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


package org.apache.webapp.admin.resources;

import java.io.IOException;
import java.util.Locale;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import javax.management.MBeanServer;
import javax.management.ObjectName;
import org.apache.struts.util.MessageResources;
import org.apache.webapp.admin.ApplicationServlet;


/**
 * <p>Implementation of <strong>Action</strong> that deletes the
 * specified set of resource links entries.</p>
 *
 * @author Amy Roh
 * @version $Revision: 479038 $ $Date: 2006-11-24 16:01:21 -0700 (Fri, 24 Nov 2006) $
 * @since 4.1
 */

public final class DeleteResourceLinksAction extends Action {


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
            return (mapping.findForward("List Resource Links Setup"));
        }

        // Check the transaction token
        if (!isTokenValid(request)) {
            response.sendError
                (HttpServletResponse.SC_BAD_REQUEST,
                 resources.getMessage(locale, "users.error.token"));
            return (null);
        }

        // Perform any extra validation that is required
        ResourceLinksForm resourceLinksForm = (ResourceLinksForm) form;
        String resourceLinks[] = resourceLinksForm.getResourceLinks();
        if (resourceLinks == null) {
            resourceLinks = new String[0];
        }

        // Perform "Delete Resource Link" transactions as required
        try {
            
            String resourcetype = resourceLinksForm.getResourcetype();
            String path = resourceLinksForm.getPath();
            String host = resourceLinksForm.getHost();
            
            ObjectName dname = null;

            String signature[] = new String[1];
            signature[0] = "java.lang.String";
            Object params[] = new String[1];
            
            for (int i = 0; i < resourceLinks.length; i++) {
                ObjectName oname = new ObjectName(resourceLinks[i]);
                String domain = oname.getDomain();  
                dname = ResourceUtils.getNamingResourceObjectName(domain,
                            resourcetype, path, host);
                params[0] = oname.getKeyProperty("name");
                mserver.invoke(dname, "removeResourceLink",
                               params, signature);
            }

        } catch (Throwable t) {

            getServlet().log
                (resources.getMessage(locale, "users.error.invoke",
                                      "removeResource"), t);
            response.sendError
                (HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                 resources.getMessage(locale, "users.error.invoke",
                                      "removeResource"));
            return (null);

        }

        // Proceed to the list envEntrys screen
        return (mapping.findForward("ResourceLinks List Setup"));

    }

}
