#pragma once


#include <string>
namespace lsp { 
/**
 * The <code>IProgressMonitor</code> interface is implemented
 * by objects that monitor the progress of an activity; the methods
 * in this interface are invoked by code that performs the activity.
 * <p>
 * All activity is broken down into a linear sequence of tasks against
 * which progress is reported. When a task begins, a <code>beginTask(const wstring&, int)
 * </code> notification is reported, followed by any number and mixture of
 * progress reports (<code>worked()</code>) and subtask notifications
 * (<code>subTask(const wstring&)</code>).  When the task is eventually completed, a
 * <code>done()</code> notification is reported.  After the <code>done()</code>
 * notification, the progress monitor cannot be reused;  i.e., <code>
 * beginTask(const wstring&, int)</code> cannot be called again after the call to
 * <code>done()</code>.
 * </p>
 * <p>
 * A request to cancel an operation can be signaled using the
 * <code>setCanceled</code> method.  Operations taking a progress
 * monitor are expected to poll the monitor (using <code>isCanceled</code>)
 * periodically and abort at their earliest convenience.  Operation can however
 * choose to ignore cancelation requests.
 * </p>
 * <p>
 * Since notification is synchronous with the activity itself, the listener should
 * provide a fast and robust implementation. If the handling of notifications would
 * involve blocking operations, or operations which might throw uncaught exceptions,
 * the notifications should be queued, and the actual processing deferred (or perhaps
 * delegated to a separate thread).
 * </p>
 * <p>
 * Clients may implement this interface.
 * </p>
 */
	class  IProgressMonitor {
	public:
		virtual ~IProgressMonitor()
		{
		}

		/** Constant indicating an unknown amount of work.
		 */

		const static int UNKNOWN = -1;

	/**
		* Notifies that the main task is beginning.  This must only be called once
		* on a given progress monitor instance.
		*
		* @param name the name (or description) of the main task
		* @param totalWork the total number of work units into which
		*  the main task is been subdivided. If the value is <code>UNKNOWN</code>
		*  the implemenation is free to indicate progress in a way which
		*  doesn't require the total number of work units in advance.
		*/
		virtual void beginTask(void* , int totalWork)
		{

		};
		/**
		 * Notifies that the work is done; that is, either the main task is completed
		 * or the user canceled it. This method may be called more than once
		 * (implementations should be prepared to handle this case).
		 */

		virtual void endTask(void*, int totalWork)
		{

		}

		virtual void done(void*) = 0;

		/**
		 * Internal method to handle scaling correctly. This method
		 * must not be called by a client. Clients should
		 * always use the method </code>worked(int)</code>.
		 */
		virtual void internalWorked(double work)
		{

		}
		/**
		 * Returns whether cancelation of current operation has been requested.
		 * Long-running operations should poll to see if cancelation
		 * has been requested.
		 *
		 * @return <code>true</code> if cancellation has been requested,
		 *    and <code>false</code> otherwise
		 * @see #setCanceled
		 */
		virtual bool isCanceled() = 0;
		/**
		 * Sets the cancel state to the given value.
		 *
		 * @param value <code>true</code> indicates that cancelation has
		 *     been requested (but not necessarily acknowledged);
		 *     <code>false</code> clears this flag
		 *
		 * @see #isCanceled
		 */
		virtual void setCanceled(bool value) = 0;
		/**
		 * Sets the task name to the given value. This method is used to
		 * restore the task label after a nested operation was executed.
		 * Normally there is no need for clients to call this method.
		 *
		 * @param name the name (or description) of the main task
		 * @see #beginTask(java.lang.const wstring&, int)
		 */
		virtual void setTaskName(void*)
		{

		};
		/**
		 * Notifies that a subtask of the main task is beginning.
		 * Subtasks are optional; the main task might not have subtasks.
		 *
		 * @param name the name (or description) of the subtask
		 */
		virtual void subTask(void* )
		{

		}
		/**
		 * Notifies that a given number of work unit of the main task
		 * has been completed. Note that this amount represents an
		 * installment, as opposed to a cumulative amount of work done
		 * to date.
		 *
		 * @param work the number of work units just completed
		 */
		virtual void worked(int work)
		{

		};
		
		virtual void catch_exception(void*) = 0;
	};
}


