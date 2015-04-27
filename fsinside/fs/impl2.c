struct fuse_operations {
	/** Get file attributes.
	 *
	 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
	 * ignored.	 The 'st_ino' field is ignored except if the 'use_ino'
	 * mount option is given.
	 */
	struct stat {
		unsigned long	st_dev;		/* Device.  */
		unsigned long	st_ino;		/* File serial number.  */
		unsigned int	st_mode;	/* File mode.  */
		unsigned int	st_nlink;	/* Link count.  */
		unsigned int	st_uid;		/* User ID of the file's owner.  */
		unsigned int	st_gid;		/* Group ID of the file's group. */
		unsigned long	st_rdev;	/* Device number, if device.  */
		unsigned long	__pad1;
		long		st_size;	/* Size of file, in bytes.  */
		int		st_blksize;	/* Optimal block size for I/O.  */
		int		__pad2;
		long		st_blocks;	/* Number 512-byte blocks allocated. */
		long		st_atime;	/* Time of last access.  */
		unsigned long	st_atime_nsec;
		long		st_mtime;	/* Time of last modification.  */
		unsigned long	st_mtime_nsec;
		long		st_ctime;	/* Time of last status change.  */
		unsigned long	st_ctime_nsec;
		unsigned int	__unused4;
		unsigned int	__unused5;
	};
	// find file status using given path -> put the info into stbuf
	int (*getattr) (const char *path, struct stat *stbuf);

	/** Create a directory 
	 *
	 * Note that the mode argument may not have the type specification
	 * bits set, i.e. S_ISDIR(mode) can be false.  To obtain the
	 * correct directory type bits use  mode|S_IFDIR
	 * */
	// make a directory
	int (*mkdir) (const char *path, mode_t mode);

	// unlink the file with the given path
	int (*unlink) (const char *path);

	// remove a directory with the given path
	int (*rmdir) (const char *path);

	// rename a file
	int (*rename) (const char *before, const char *after);

	// chage the size of a file
	int (*truncate) (const char *path, off_t size);

	/** File open operation
	 *
	 * No creation (O_CREAT, O_EXCL) and by default also no
	 * truncation (O_TRUNC) flags will be passed to open(). If an
	 * application specifies O_TRUNC, fuse first calls truncate()
	 * and then open(). Only if 'atomic_o_trunc' has been
	 * specified and kernel version is 2.6.24 or later, O_TRUNC is
	 * passed on to open.
	 *
	 * Unless the 'default_permissions' mount option is given,
	 * open should check if the operation is permitted for the
	 * given flags. Optionally open may also return an arbitrary
	 * filehandle in the fuse_file_info structure, which will be
	 * passed to all file operations.
	 *
	 * Changed in version 2.2
	 */
	struct fuse_file_info {
		/** Open flags.	 Available in open() and release() */
		int flags;

		/** Old file handle, don't use */
		unsigned long fh_old;

		/** In case of a write operation indicates if this was caused by a
			writepage */
		int writepage;

		/** Can be filled in by open, to use direct I/O on this file.
			Introduced in version 2.4 */
		unsigned int direct_io : 1;

		/** Can be filled in by open, to indicate, that cached file data
			need not be invalidated.  Introduced in version 2.4 */
		unsigned int keep_cache : 1;

		/** Indicates a flush operation.  Set in flush operation, also
			maybe set in highlevel lock operation and lowlevel release
			operation.	Introduced in version 2.6 */
		unsigned int flush : 1;

		/** Can be filled in by open, to indicate that the file is not
			seekable.  Introduced in version 2.8 */
		unsigned int nonseekable : 1;

		/* Indicates that flock locks for this file should be
		   released.  If set, lock_owner shall contain a valid value.
		   May only be set in ->release().  Introduced in version
		   2.9 */
		unsigned int flock_release : 1;

		/** Padding.  Do not use*/
		unsigned int padding : 27;

		/** File handle.  May be filled in by filesystem in open().
			Available in all other file operations */
		uint64_t fh;

		/** Lock owner id.  Available in locking operations and flush */
		uint64_t lock_owner;
	};
	int (*open) (const char *path, struct fuse_file_info *info);

	/** Read data from an open file
	 *
	 * Read should return exactly the number of bytes requested except
	 * on EOF or error, otherwise the rest of the data will be
	 * substituted with zeroes.	 An exception to this is when the
	 * 'direct_io' mount option is specified, in which case the return
	 * value of the read system call will reflect the return value of
	 * this operation.
	 *
	 * Changed in version 2.2
	 */
	int (*read) (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info);

	/** Write data to an open file
	 *
	 * Write should return exactly the number of bytes requested
	 * except on error.	 An exception to this is when the 'direct_io'
	 * mount option is specified (see read operation).
	 *
	 * Changed in version 2.2
	 */
	int (*write) (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info);

	/** Release an open file
	 *
	 * Release is called when there are no more references to an open
	 * file: all file descriptors are closed and all memory mappings
	 * are unmapped.
	 *
	 * For every open() call there will be exactly one release() call
	 * with the same flags and file descriptor.	 It is possible to
	 * have a file opened more than once, in which case only the last
	 * release will mean, that no more reads/writes will happen on the
	 * file.  The return value of release is ignored.
	 *
	 * Changed in version 2.2
	 */
	int (*release) (const char *path, struct fuse_file_info *info);

	/** Open directory
	 *
	 * Unless the 'default_permissions' mount option is given,
	 * this method should check if opendir is permitted for this
	 * directory. Optionally opendir may also return an arbitrary
	 * filehandle in the fuse_file_info structure, which will be
	 * passed to readdir, closedir and fsyncdir.
	 *
	 * Introduced in version 2.3
	 */
	int (*opendir) (const char *, struct fuse_file_info *);

	/** Read directory
	 *
	 * This supersedes the old getdir() interface.  New applications
	 * should use this.
	 *
	 * The filesystem may choose between two modes of operation:
	 *
	 * 1) The readdir implementation ignores the offset parameter, and
	 * passes zero to the filler function's offset.  The filler
	 * function will not return '1' (unless an error happens), so the
	 * whole directory is read in a single readdir operation.  This
	 * works just like the old getdir() method.
	 *
	 * 2) The readdir implementation keeps track of the offsets of the
	 * directory entries.  It uses the offset parameter and always
	 * passes non-zero offset to the filler function.  When the buffer
	 * is full (or an error happens) the filler function will return
	 * '1'.
	 *
	 * Introduced in version 2.3
	 */
	/** Function to add an entry in a readdir() operation
	 *
	 * @param buf the buffer passed to the readdir() operation
	 * @param name the file name of the directory entry
	 * @param stat file attributes, can be NULL
	 * @param off offset of the next entry or zero
	 * @return 1 if buffer is full, zero otherwise
	 */
	typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
					const struct stat *stbuf, off_t off);
	int (*readdir) (const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *info);

	/** Release directory
	 *
	 * Introduced in version 2.3
	 */
	int (*releasedir) (const char *, struct fuse_file_info *);

	/**
	 * Initialize filesystem
	 *
	 * The return value will passed in the private_data field of
	 * fuse_context to all file operations and as a parameter to the
	 * destroy() method.
	 *
	 * Introduced in version 2.3
	 * Changed in version 2.6
	 */
	struct fuse_conn_info {
		/**
		 * Major version of the protocol (read-only)
		 */
		unsigned proto_major;

		/**
		 * Minor version of the protocol (read-only)
		 */
		unsigned proto_minor;

		/**
		 * Is asynchronous read supported (read-write)
		 */
		unsigned async_read;

		/**
		 * Maximum size of the write buffer
		 */
		unsigned max_write;

		/**
		 * Maximum readahead
		 */
		unsigned max_readahead;

		/**
		 * Capability flags, that the kernel supports
		 */
		unsigned capable;

		/**
		 * Capability flags, that the filesystem wants to enable
		 */
		unsigned want;

		/**
		 * Maximum number of backgrounded requests
		 */
		unsigned max_background;

		/**
		 * Kernel congestion threshold parameter
		 */
		unsigned congestion_threshold;

		/**
		 * For future use.
		 */
		unsigned reserved[23];
	};
	void *(*init) (struct fuse_conn_info *conn);

	/**
	 * Clean up filesystem
	 *
	 * Called on filesystem exit.
	 *
	 * Introduced in version 2.3
	 */
	void (*destroy) (void *user_data);

	/**
	 * Create and open a file
	 *
	 * If the file does not exist, first create it with the specified
	 * mode, and then open it.
	 *
	 * If this method is not implemented or under Linux kernel
	 * versions earlier than 2.6.15, the mknod() and open() methods
	 * will be called instead.
	 *
	 * Introduced in version 2.5
	 */
	int (*create) (const char *path, mode_t mode, struct fuse_file_info *info);

	unsigned int flag_nullpath_ok:1;
	unsigned int flag_nopath:1;
	unsigned int flag_utime_omit_ok:1;
	unsigned int flag_reserved:29;
};
